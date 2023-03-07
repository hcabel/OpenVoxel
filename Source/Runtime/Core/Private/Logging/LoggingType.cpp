#include "Logging/LoggingType.h"

namespace Verbosity
{
	const char* ToString(Type Verbosity)
	{
		switch (Verbosity)
		{
		case Type::Unknown:
			return "Unknown";
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
}