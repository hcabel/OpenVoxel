#include "PlatformTime.h"

#include <GLFW/glfw3.h>

#pragma region Statics
float PlatformTime::s_BeginFrameTime = 0.0f;
float PlatformTime::s_TimeStep = 0.0f;
#pragma endregion

float PlatformTime::GetTime()
{
	return (glfwGetTime());
}

float PlatformTime::GetTimeStep()
{
	return s_TimeStep;
}

void PlatformTime::CalculateNewTiming()
{
	const float currentTime = GetTime();
	s_TimeStep = currentTime - s_BeginFrameTime;
	s_BeginFrameTime = currentTime;
}

void PlatformTime::Init()
{
	s_BeginFrameTime = GetTime();
}