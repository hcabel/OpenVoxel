#pragma once

#include "Generic/GenericPlatformTime.h"

class WindowsPlatformTime : public GenericPlatformTime
{
public:
	WindowsPlatformTime() = delete;
	~WindowsPlatformTime() = delete;

	static float GetTime();
	static float GetTimeStep();

	static void CalculateNewTiming();

	static void Init();

private:
	static float s_BeginFrameTime;
	static float s_TimeStep;

};

// Allow the platform to override the generic implementation.
typedef WindowsPlatformTime PlatformTime;
