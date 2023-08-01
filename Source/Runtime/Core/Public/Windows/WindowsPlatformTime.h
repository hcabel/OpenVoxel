#pragma once

#include "Core_API.h"
#include "Generic/GenericPlatformTime.h"

#include <string>

class CORE_API WindowsPlatformTime : public GenericPlatformTime
{
public:
	WindowsPlatformTime() = delete;
	~WindowsPlatformTime() = delete;

	static float GetTime();
	static float GetTimeStep();

	static void CalculateNewTiming();

	/**
	 * Get current UTC date in a specific format.
	 * 
	 * \param format The format to use. @see https://cplusplus.com/reference/ctime/strftime
	 * \return The date in the specified format.
	 */
	static std::string GetDate(const char* format);

	static void Init();

private:
	/**
	 * Calculate the buffer size needed to store the date in a specific format.
	 * @see https://cplusplus.com/reference/ctime/strftime
	 * 
	 * \param format The format to use.
	 * \return The size of the buffer.
	 */
	static size_t CalculateDateFormatBufferSize(const char* format);

private:
	static float s_BeginFrameTime;
	static float s_TimeStep;

};
