#include "Windows/WindowsPlatformTime.h"
#include "CoreGlobals.h"

#include <GLFW/glfw3.h>
#include <vector>

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

std::string WindowsPlatformTime::GetDate(const char* format)
{
	time_t now = time(nullptr);
	tm *nowLocal = localtime(&now);

	size_t expectedSize = CalculateDateFormatBufferSize(format);
	std::vector<char> buffer(expectedSize + 1);
	size_t realSize = strftime(buffer.data(), buffer.size() * sizeof(char), format, nowLocal);
	if (expectedSize != realSize)
		OV_LOG(CoreLog, Error, "Wrong date format size, expected {:d}, got {:d}", expectedSize, realSize);

	return (std::string(buffer.data()));
}

void WindowsPlatformTime::Init()
{
	s_BeginFrameTime = GetTime();
}

size_t WindowsPlatformTime::CalculateDateFormatBufferSize(const char *format)
{
	size_t size = 0;
	size_t i = 0;
	while (format[i])
	{
		if (format[i] == '%')
		{
			i++; // Skip the '%'
			switch (format[i])
			{
			// see https://cplusplus.com/reference/ctime/strftime
			case 'a': size += 2; break;
			case 'A': size += 9; break;
			case 'b': size += 3; break;
			case 'B': size += 9; break;
			case 'c': size += 24; break;
			case 'C': size += 2; break;
			case 'd': size += 2; break;
			case 'D': size += 8; break;
			case 'e': size += 2; break;
			case 'F': size += 10; break;
			case 'g': size += 2; break;
			case 'G': size += 4; break;
			case 'h': size += 3; break;
			case 'H': size += 2; break;
			case 'I': size += 2; break;
			case 'j': size += 3; break;
			case 'm': size += 2; break;
			case 'M': size += 2; break;
			case 'n': size += 1; break;
			case 'p': size += 2; break;
			case 'r': size += 11; break;
			case 'R': size += 5; break;
			case 'S': size += 2; break;
			case 't': size += 1; break;
			case 'T': size += 8; break;
			case 'u': size += 1; break;
			case 'U': size += 2; break;
			case 'V': size += 2; break;
			case 'w': size += 1; break;
			case 'W': size += 2; break;
			case 'x': size += 8; break;
			case 'X': size += 8; break;
			case 'y': size += 2; break;
			case 'Y': size += 4; break;
			case 'z': size += 5; break;
			case 'Z': size += 3; break;
			case '%': size += 1; break;
			default:
				OV_LOG(CoreLog, Error, "Unknown format specifier '{:c}' in date format string '{:s}'", format[i], format);
				size += 2; // Add the '%' and the unknown specifier
				break;
			}
		}
		else
			size++;
		i++;
	}
	return size;
}
