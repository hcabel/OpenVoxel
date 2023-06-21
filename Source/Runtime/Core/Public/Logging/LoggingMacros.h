#pragma once

#include "MacrosHelper.h"
#include "Logging/LoggingType.h"
#include "Logging/Logger.h"

// create a more explicit logging macro if logging is enabled
#define WITH_LOGGING !NO_LOGGING

#if NO_LOGGING

# define OV_LOG(LogVerbosity, Category, Format, ...) \
	if (Verbosity::LogVerbosity == Verbosity::Fatal) \
		assert(false);

# define DECLARE_LOG_CATEGORY(CategoryName) EMPTY_MACRO
# define DEFINE_LOG_CATEGORY(CategoryName) EMPTY_MACRO

#else // !NO_LOGGING || WITH_LOGGING

# define OV_LOG(LogVerbosity, Category, Format, ...) \
	{ \
		STATIC_CHECK_TYPE(Verbosity::Type, Verbosity::LogVerbosity); \
		STATIC_CHECK_TYPE(LogCategory, Category); \
		Logger::Log(Verbosity::LogVerbosity, Category, std::format(Format, __VA_ARGS__)); \
	}

# define OV_LOG_ARRAY(LogVerbosity, Category, Array, Format, ...) \
	FOR_EACH_LOOP(Entry, Array) \
		OV_LOG(LogVerbosity, Category, Format, Entry, __VA_ARGS__); \

# define DECLARE_LOG_CATEGORY(CategoryName) extern LogCategory CategoryName;
# define DEFINE_LOG_CATEGORY(CategoryName) LogCategory CategoryName(#CategoryName);

#endif // !NO_LOGGING || WITH_LOGGING

/* MACRO THAT DOES NOT CHANGE WHEN LOGGING IS DISABLED */

// Log if condition is true
# define OV_LOG_IF(Condition, LogVerbosity, Category, Format, ...) \
	if (Condition) \
		OV_LOG(LogVerbosity, Category, Format, __VA_ARGS__)
