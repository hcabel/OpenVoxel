#pragma once

#include "Core_API.h"

#include <stdint.h>
#include <string>

namespace Verbosity
{
	enum Type : uint8_t
	{
		Unknown = 0,

		/* Log errors onto file and console, in RED and crash the application */
		Fatal = 1,
		/* Log errors onto file and console, in RED */
		Error = 2,
		/* Log warnings onto file and console, in YELLOW */
		Warning = 3,
		/* Log info onto console, in WHITE */
		Display = 4,
		/* Log info onto file and console, in WHITE */
		Verbose = 5,
		/* Log info onto file only */
		VeryVerbose = 6,
	};

	CORE_API const char* ToString(Type Verbosity);
	CORE_API Type ToType(const std::string_view& verbosity);
}

class CORE_API LogCategory
{
public:
	LogCategory(std::string &&categoryName)
		: m_Name(std::move(categoryName))
	{}

	// Remove copy constructor and copy assignment operator
	LogCategory(const LogCategory&) = delete;
	LogCategory& operator=(const LogCategory&) = delete;

#pragma region Accessors - Get
	inline const std::string_view GetName() const { return m_Name;	}
#pragma endregion

private:
	std::string m_Name;
};
