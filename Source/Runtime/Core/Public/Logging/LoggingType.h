#pragma once

#include <stdint.h>
#include <string>

namespace Verbosity
{
	enum class Type : uint8_t
	{
		Unknown = 0,

		/* Log errors onto file and console, in RED */
		Error = 1,
		/* Log warnings onto file and console, in YELLOW */
		Warning = 2,
		/* Log info onto console, in WHITE */
		Display = 3,
		/* Log info onto file and console, in WHITE */
		Verbose = 4,
		/* Log info onto file only */
		VeryVerbose = 5,
	};

	const char* ToString(Type Verbosity);
}

class LogCategory
{
public:
	LogCategory(std::string &&categoryName)
		: m_Name(std::move(categoryName))
	{}

	// Remove copy constructor and copy assignment operator
	LogCategory(const LogCategory&) = delete;
	LogCategory& operator=(const LogCategory&) = delete;

#pragma region Accessors - Get
	const std::string_view GetName() const { return m_Name;	}
#pragma endregion

private:
	std::string m_Name;
};
