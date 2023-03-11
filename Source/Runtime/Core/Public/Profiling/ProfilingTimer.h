#pragma once

#include "Logging/LoggingMacros.h"

#include <chrono>
#include <functional>

DECLARE_LOG_CATEGORY(ProfilingLog);

class Timer
{
public:
	Timer();

	void Reset();
	std::chrono::nanoseconds Elapsed();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class ScopeTimer
{

public:
	template<typename F>
	ScopeTimer(F onEnded)
		: m_OnEnded(std::move(onEnded))
	{}
	virtual ~ScopeTimer()
	{
		m_OnEnded(m_Timer.Elapsed());
	}

protected:
	Timer m_Timer;
	std::function<void(std::chrono::nanoseconds)> m_OnEnded;
};

class ConsoleScopeTimer : ScopeTimer
{
public:
	ConsoleScopeTimer(const char* timerName)
		: ScopeTimer([timerName](std::chrono::nanoseconds elapsed)
			{
				// NanoSeconds to MilliSeconds
				float time = elapsed.count() * 0.001f * 0.001f;
				OV_LOG(Verbose, ProfilingLog, "ScopeTimer: {} - {:.2f}ms", timerName, time);
			})
	{}

};
