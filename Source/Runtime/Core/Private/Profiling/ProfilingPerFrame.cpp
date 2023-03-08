#include "Profiling/ProfilingPerFrame.h"

std::unordered_map<std::string, PerFrameProfilingData> PerFrameProfiler::s_ProfilingDatas;

void PerFrameProfilingData::AddCall(std::chrono::nanoseconds timeMicroSeconds)
{
	m_TimeMicroSeconds.push_back(timeMicroSeconds);
	m_Min = std::chrono::nanoseconds(glm::min<uint64_t>(m_Min.count(), timeMicroSeconds.count()));
	m_Max = std::chrono::nanoseconds(glm::max<uint64_t>(m_Max.count(), timeMicroSeconds.count()));
}

std::chrono::nanoseconds PerFrameProfilingData::CalculateAverage() const
{
	if (m_TimeMicroSeconds.size() == 0)
		return (std::chrono::nanoseconds(0));

	std::chrono::nanoseconds sum{ 0 };
	for (auto& time : m_TimeMicroSeconds)
		sum += time;
	return (sum / m_TimeMicroSeconds.size());
}

std::chrono::nanoseconds PerFrameProfilingData::CalculateTotal() const
{
	if (m_TimeMicroSeconds.size() == 0)
		return (std::chrono::nanoseconds(0));

	std::chrono::nanoseconds sum{ 0 };
	for (auto& time : m_TimeMicroSeconds)
		sum += time;
	return (sum);
}

void PerFrameProfiler::Report(std::string_view categoryName, std::chrono::nanoseconds &timeMicroSeconds)
{
	Report(categoryName.data(), timeMicroSeconds);
}

void PerFrameProfiler::Report(const char* categoryName, std::chrono::nanoseconds &timeMicroSeconds)
{
	if (s_ProfilingDatas.find(categoryName) == s_ProfilingDatas.end())
		s_ProfilingDatas[categoryName] = PerFrameProfilingData();
	s_ProfilingDatas[categoryName]
		.AddCall(timeMicroSeconds);
}

void PerFrameProfiler::ResetAllData()
{
	s_ProfilingDatas.clear();
}

void PerFrameProfiler::ResetData(std::string_view categoryName)
{
	ResetData(categoryName.data());
}

void PerFrameProfiler::ResetData(const char* categoryName)
{
	s_ProfilingDatas.erase(categoryName);
}

const PerFrameProfilingData& PerFrameProfiler::GetData(std::string_view categoryName)
{
	return (s_ProfilingDatas.at(categoryName.data()));
}