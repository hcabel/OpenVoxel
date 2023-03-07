#pragma once

#include "Logging/LoggingType.h"

#include <iostream>
#include <stdio.h>
#include <format>

/**
 * Static Class that log message onto console and/or file.
 * TODO: Add file logging
 * TODO: Add thread safe logging (using a queue probably)
 */
class Logger final
{
public:
	static void Log(Verbosity::Type verbosity, LogCategory& category, std::string message);

private:
	static void LogOntoConsole(std::string_view logMessage);
};
