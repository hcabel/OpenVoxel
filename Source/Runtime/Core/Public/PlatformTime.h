#pragma once

/**
 * Static class that handle the time for the current platform.
 * (CURRENTLY ONLY ON WINDOWS)
 */
class PlatformTime
{

public:
	PlatformTime() = delete;
	~PlatformTime() = delete;

	static float GetTime();
	static float GetTimeStep();

	static void CalculateNewTiming();

	static void Init();

private:
	static float s_BeginFrameTime;
	static float s_TimeStep;

};
