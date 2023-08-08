#include "Profiling/ProfilingPerFrame.h"

std::unordered_map<std::string, PerFrameProfilingData> PerFrameProfilerStorage::s_ProfilingDatas;

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

void PerFrameProfilerStorage::Report(std::string_view categoryName, std::chrono::nanoseconds& timeMicroSeconds)
{
	Report(categoryName.data(), timeMicroSeconds);
}

void PerFrameProfilerStorage::Report(const char* categoryName, std::chrono::nanoseconds& timeMicroSeconds)
{
	if (s_ProfilingDatas.find(categoryName) == s_ProfilingDatas.end())
		s_ProfilingDatas[categoryName] = PerFrameProfilingData();
	s_ProfilingDatas[categoryName]
		.AddCall(timeMicroSeconds);
}

void PerFrameProfilerStorage::ClearAllData()
{
	s_ProfilingDatas.clear();
}

void PerFrameProfilerStorage::ClearData(std::string_view categoryName)
{
	ClearData(categoryName.data());
}

void PerFrameProfilerStorage::ClearData(const char* categoryName)
{
	s_ProfilingDatas.erase(categoryName);
}

const PerFrameProfilingData& PerFrameProfilerStorage::GetData(std::string_view categoryName)
{
	return (s_ProfilingDatas.at(categoryName.data()));
}