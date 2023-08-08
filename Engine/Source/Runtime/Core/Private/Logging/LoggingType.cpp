#include "Logging/LoggingType.h"

namespace Verbosity
{
	const char* ToString(Type Verbosity)
	{
		switch (Verbosity)
		{
		case Type::Unknown:
			return "Unknown";
		case Type::Fatal:
			return "Fatal";
		case Type::Error:
			return "Error";
		case Type::Warning:
			return "Warning";
		case Type::Display:
			return "Display";
		case Type::Verbose:
			return "Verbose";
		case Type::VeryVerbose:
			return "VeryVerbose";
		default:
			return "Unknown";
		}
	}

	Verbosity::Type ToType(const std::string_view& verbosity)
	{
		if (verbosity == "Unknown")
			return Verbosity::Unknown;
		if (verbosity == "Fatal")
			return Verbosity::Fatal;
		if (verbosity == "Error")
			return Verbosity::Error;
		if (verbosity == "Warning")
			return Verbosity::Warning;
		if (verbosity == "Display")
			return Verbosity::Display;
		if (verbosity == "Verbose")
			return Verbosity::Verbose;
		if (verbosity == "VeryVerbose")
			return Verbosity::VeryVerbose;
		return Verbosity::Unknown;
	}
}