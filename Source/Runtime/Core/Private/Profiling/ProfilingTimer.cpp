#include "Profiling/ProfilingTimer.h"

DEFINE_LOG_CATEGORY(ProfilingLog);

Timer::Timer()
{
	Start();
}

void Timer::Reset()
{
	m_Start = std::chrono::high_resolution_clock::now();
}

std::chrono::nanoseconds Timer::Elapsed()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start);
}