#pragma once

#include "MacrosHelper.h"
#include "Logging/LoggingType.h"
#include "Logging/Logger.h"

#if NO_LOGGING

# define OV_LOG(LogVerbosity, Category, Format, ...) \
	if (Verbosity::LogVerbosity == Verbosity::Fatal) \
		assert(false);
# define DECLARE_LOG_CATEGORY(CategoryName) EMPTY_MACRO
# define DEFINE_LOG_CATEGORY(CategoryName) EMPTY_MACRO

#else // !NO_LOGGING || WITH_LOGGING
// create a more explicit logging macro if logging is enabled
#define WITH_LOGGING 1

# define OV_LOG(LogVerbosity, Category, Format, ...) \
	{ \
		STATIC_CHECK_TYPE(Verbosity::Type, Verbosity::LogVerbosity); \
		STATIC_CHECK_TYPE(LogCategory, Category); \
		Logger::Log(Verbosity::LogVerbosity, Category, std::format(Format, __VA_ARGS__)); \
	}

# define DECLARE_LOG_CATEGORY(CategoryName) extern class LogCategory CategoryName;
# define DEFINE_LOG_CATEGORY(CategoryName) LogCategory CategoryName(#CategoryName);

#endif // !NO_LOGGING || WITH_LOGGING