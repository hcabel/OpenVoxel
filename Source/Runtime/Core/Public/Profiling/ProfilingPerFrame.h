#pragma once

#include "Profiling/ProfilingTimer.h"

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

/** Perframe datas collected by per frame timers */
struct PerFrameProfilingData
{

public:
	void AddCall(std::chrono::nanoseconds timeMicroSeconds);

	/** Return how many time this category has been called this frame */
	size_t GetCallCount() const { return m_TimeMicroSeconds.size(); }
	/** Return the minimum call duration */
	std::chrono::nanoseconds GetMin() const { return (m_Min.count() == UINT64_MAX ? std::chrono::nanoseconds(0) : m_Min); }
	/** Return the maximum call duration */
	std::chrono::nanoseconds GetMax() const { return (m_Max); }
	/** Return the average call duration */
	std::chrono::nanoseconds CalculateAverage() const;
	/** Return the total call duration */
	std::chrono::nanoseconds CalculateTotal() const;

protected:
	std::vector<std::chrono::nanoseconds> m_TimeMicroSeconds;
	std::chrono::nanoseconds m_Min{ UINT64_MAX };
	std::chrono::nanoseconds m_Max{ 0 };
};

/**
 * This static cast collect/store data from PerFrame timer and store them.
 */
class PerFrameProfilerStorage final
{
public:
	static void Report(std::string_view categoryName, std::chrono::nanoseconds &timeMicroSeconds);
	static void Report(const char* categoryName, std::chrono::nanoseconds &timeMicroSeconds);

	static void ClearAllData();
	static void ClearData(std::string_view categoryName);
	static void ClearData(const char* categoryName);

	static const PerFrameProfilingData& GetData(std::string_view categoryName);

private:
	static std::unordered_map<std::string, PerFrameProfilingData> s_ProfilingDatas;
};

/**
 * Create a timer that will track the execution of the current scope.
 * When done this timer will report his data to the PerFrameProfiler.
 */
class ScopeTimerPerFrame : public ScopeTimer
{

public:
	ScopeTimerPerFrame(const char* categoryName)
		: ScopeTimer([categoryName](std::chrono::nanoseconds elapsed)
			{
				PerFrameProfilerStorage::Report(categoryName, elapsed);
			})
	{}

};
