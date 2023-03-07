#pragma once

#include "Logging/LoggingType.h"
#include "Logging/Logger.h"

#if OV_DEBUG
# define OV_LOGGING
#endif

#ifdef OV_LOGGING

# define OV_LOG(LogVerbosity, Category, Format, ...) \
	static_assert(std::is_same<Verbosity::Type, decltype(LogVerbosity)>::value, "Verbosity must be a Verbosity::Type"); \
	static_assert(std::is_same<LogCategory, decltype(Category)>::value, "Category must be a LogCategory"); \
	Logger::Log(LogVerbosity, Category, std::format(Format, __VA_ARGS__))

# define DECLARE_LOG_CATEGORY(CategoryName) extern class LogCategory CategoryName;
# define DEFINE_LOG_CATEGORY(CategoryName) LogCategory CategoryName(#CategoryName);

#else
// Remove the logging macros if logging is disabled

#endif
