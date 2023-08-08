#include "Logging/Logger.h"
#include "HAL/Time.h"
#include "Path.h"
#include "CoreGlobals.h"

#include <assert.h>
#include <format>

std::unique_ptr<File> Logger::s_LogFile = nullptr;

void Logger::Log(Verbosity::Type verbosity, LogCategory& category, std::string message)
{
	// trim message trailing whitespace characters
	size_t last = message.find_last_not_of(" \t\n\r\f\v");
	last = (last == std::string::npos ? message.length() - 1 : last + 1);
	std::string cleanMessage = message.substr(0, last);
	// remove all newline characters
	cleanMessage.erase(std::remove(cleanMessage.begin(), cleanMessage.end(), '\n'), cleanMessage.end());

	std::string logMessage = std::format("[{:s}]: {:s}: {:s}", category.GetName(), Verbosity::ToString(verbosity), cleanMessage);
#ifdef OV_DEBUG
	LogOntoConsole(logMessage);
#endif
	LogOntoFile(logMessage);

	if (verbosity == Verbosity::Fatal)
		assert(false);
}

void Logger::LogOntoConsole(std::string_view logMessage)
{
	std::cout << logMessage << std::endl;
}

void Logger::LogOntoFile(std::string_view logMessage)
{
	// Get the log file handle, if none exists, create it
	if (s_LogFile == nullptr)
	{
		std::string logFilePath = GetLogFilePath();
		s_LogFile = File::OpenUnique(logFilePath, std::ios_base::app);
	}
	else
		s_LogFile->Open(std::ios_base::app);

	*s_LogFile << logMessage << std::endl;
	s_LogFile->Close();
}

std::string Logger::GetLogFilePath()
{
	// File name format: log_YYYY-MM-DD_HH-MM-SS.txt
	return (Path::GetLogDirectoryPath() + std::format("log_{:s}.txt", Time::GetDate("%F_%H-%M-%S")));
}
