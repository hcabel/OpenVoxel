#include "Windows/WindowsPlatformTime.h"

#include <GLFW/glfw3.h>

#pragma region Statics
float WindowsPlatformTime::s_BeginFrameTime = 0.0f;
float WindowsPlatformTime::s_TimeStep = 0.0f;
#pragma endregion

float WindowsPlatformTime::GetTime()
{
	float time = static_cast<float>(glfwGetTime());
	if (time == 0.0f)
		glfwInit();
	return (time);
}

float WindowsPlatformTime::GetTimeStep()
{
	return s_TimeStep;
}

void WindowsPlatformTime::CalculateNewTiming()
{
	const float currentTime = GetTime();
	s_TimeStep = currentTime - s_BeginFrameTime;
	s_BeginFrameTime = currentTime;
}

void WindowsPlatformTime::Init()
{
	s_BeginFrameTime = GetTime();
}
