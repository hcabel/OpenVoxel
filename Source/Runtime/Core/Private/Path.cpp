#include "Path.h"

#include "Singleton.h"
#include "HAL/PlatformFileSystem.h"

Path::DataCache Path::s_Data;

Path::Path()
{}

Path::Path(const Path& ref)
{
	Copy(ref);
}

Path::~Path()
{
	m_Path.clear();
}

Path& Path::Copy(const Path& rhs)
{
	m_Path = rhs.m_Path;
	return (*this);
}

Path& Path::Append(const Path& rhs)
{
	m_Path.insert(m_Path.end(), rhs.m_Path.begin(), rhs.m_Path.end());
	return (*this);
}

Path& Path::Prepend(const Path& rhs)
{
	m_Path.insert(m_Path.begin(), rhs.m_Path.begin(), rhs.m_Path.end());
	return (*this);
}

Path& Path::TrimSegments(size_type start, size_type end)
{
	if (start > end)
		return (*this);
	if (start >= m_Path.size())
		return (*this);

	if (end >= m_Path.size())
		end = m_Path.size() - 1;

	m_Path.erase(m_Path.begin() + start, m_Path.begin() + end + 1);
	return (*this);
}

Path& Path::TrimTarget()
{
	auto targetSegment = m_Path.end() - 1;
	if (targetSegment->find('.') != std::string::npos)
		m_Path.erase(targetSegment);

	return (*this);
}

Path& Path::InsertSegment(size_type segmentIndex, const std::string_view segment)
{
	if (segmentIndex > m_Path.size())
		return (*this);

	m_Path.insert(m_Path.begin() + segmentIndex, std::string(segment));
	return (*this);
}

Path::size_type Path::FindSegment(const std::string_view segment) const
{
	for (size_type i = 0; i < m_Path.size(); i++)
	{
		if (m_Path[i] == segment)
			return (i);
	}
	return (size_type());
}

Path::size_type Path::RightFindSegment(const std::string_view segment) const
{
	for (size_type i = m_Path.size() - 1; i >= 0; i--)
	{
		if (m_Path[i] == segment)
			return (i);
	}
	return (-1);
}

void Path::SplitOntoSegment(const std::string_view path)
{
	size_t pathLength = path.size();

	// calculate the number of separators in the path to allocate the right amount of memory in one go
	size_t separatorCount = 0;
	for (size_t i = 0; i < pathLength; i++)
	{
		if (IsSeparator(path[i]))
			separatorCount++;
	}
	// allocate the memory
	m_Path.reserve(separatorCount + 1);

	// go through the path and split it into words
	size_t wordStartPosition;
	for (size_t i = 0; i < pathLength; i++)
	{
		wordStartPosition = i;
		while (i < pathLength && IsSeparator(path[i]) == false)
			i++;

		m_Path.push_back(std::string(path.substr(wordStartPosition, i - wordStartPosition)));
	}
}

std::string Path::GetPath(const char separator) const
{
	char buffer[MAX_PATH_LENGTH] = "";

	for (size_t i = 0; i < m_Path.size(); i++)
	{
		strncat_s(buffer, m_Path[i].c_str(), m_Path[i].size());
		if (i < m_Path.size() - 1)
			strncat_s(buffer, &separator, 1);
	}

	return (std::string(buffer));
}

const std::string_view Path::GetFileTarget() const
{
	std::string_view lastSegment = m_Path[m_Path.size() - 1];
	if (lastSegment.find_last_of('.') == std::string::npos)
		return (std::string_view());
	return (lastSegment);
}

const std::string_view Path::GetExtension() const
{
	std::string_view targetFile = GetFileTarget();
	if (targetFile.empty())
		return (std::string_view());

	return (targetFile.substr(targetFile.find_last_of('.') + 1)); // +1 to skip the '.'
}

const std::string_view Path::GetFileName() const
{
	std::string_view targetFile = GetFileTarget();
	if (targetFile.empty())
		return (std::string_view());

	return (targetFile.substr(0, targetFile.find_last_of('.')));
}

#pragma region Static - API
/**
 * Create the content of a function that will get a path from the cache
 * if the cache is empty, it will call the getter function in the PlatformFileSystem to get the path.
 * @param VariableName the name of the variable that will be used to store the path in the cache
 * @note The getter function must be named Make[VariableName]Path in the PlatformFileSystem class
 */
#define PATH_GETTER_CACHED(VariableName) \
	/* Get the VariableName from in cache */ \
	Path* VariableName = s_Data.VariableName; \
	if (VariableName == nullptr) \
	{ \
		/* if cache is empty, get the value using the getter function */ \
		VariableName = new Path(PlatformFileSystem::Make##VariableName##Path()); \
	} \
	return (*VariableName);

Path Path::GetEngineRootDirectoryPath()
{
	PATH_GETTER_CACHED(EngineRootDirectory);
}

Path Path::GetModuleDirectoryPath()
{
	PATH_GETTER_CACHED(ModuleDirectory);
}

Path Path::GetLogDirectoryPath()
{
	return (GetSavedDirectoryPath().AppendSegment("logs"));
}

Path Path::GetSavedDirectoryPath()
{
	return (GetEngineRootDirectoryPath().AppendSegment("saved\\"));
}
#pragma endregion

#undef PATH_GETTER_CACHED
