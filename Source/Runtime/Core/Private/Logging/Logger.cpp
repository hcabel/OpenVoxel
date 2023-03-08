#include "Logging/Logger.h"

void Logger::Log(Verbosity::Type verbosity, LogCategory& category, std::string message)
{
	std::string logMessage = std::format("[{}]: {}: {}", category.GetName(), Verbosity::ToString(verbosity), message);
	// TODO: Log onto the appropriate place depending on the verbosity like describe in the Verbosity enum
	LogOntoConsole(logMessage);
}

void Logger::LogOntoConsole(std::string_view logMessage)
{
	std::cout << logMessage << std::endl;
}