#pragma once

#include "Core_API.h"
#include "Logging/LoggingType.h"
#include "HAL/File.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <memory>
#include <format>

/**
 * Static Class that log message onto console and/or file.
 * TODO: Add file logging
 * TODO: Add thread safe logging (using a queue probably)
 */
class CORE_API Logger final
{
public:
	static void Log(Verbosity::Type verbosity, LogCategory& category, std::string message);

private:
	static void LogOntoConsole(std::string_view logMessage);
	static void LogOntoFile(std::string_view logMessage);
	static std::string GetLogFilePath();

private:
	static std::unique_ptr<File> s_LogFile;
};
