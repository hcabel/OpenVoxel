#pragma once

#include "Core_API.h"

/**
 * Abstract class for platform time implementation.
 */
class CORE_API GenericPlatformTime
{
public:
	GenericPlatformTime() = delete;
	~GenericPlatformTime() = delete;

	static float GetTime() { return (0.0f); }
	static float GetTimeStep() { return (0.0f); }

	static void CalculateNewTiming() {}

	static void Init() {}

protected:
	static float s_BeginFrameTime;
	static float s_TimeStep;
};
