#pragma once

#include "Core_API.h"

#include <string>
#include <vector>

/**
 * A class that represent a path.
 * Also provide some helper function to manipulate/get paths.
 */
class CORE_API Path
{
public:
	// Path are limited to 65535 segments, because path are limited in characters so even in worst case scenario we will never reach this limit
	// (could have been 255 but I want to be safe)
	using size_type = uint16_t;

public:
	Path();
	Path(const Path& ref);
	Path(const char* path) { SplitOntoSegment(std::string_view(path)); }
	Path(const std::string_view path) { SplitOntoSegment(path); }
	~Path();

#pragma region API
public:
	__forceinline Path& operator=(const Path& rhs) { return Copy(rhs); }
	__forceinline operator std::string() const { return std::move(GetPath()); }
	template<typename T>
	__forceinline Path& operator+(const T& rhs) { return Append(rhs); }

public:
	/** Create a new path by copying the rhs path */
	Path& Copy(const Path& rhs);

	/** Add a path at the end of the first path */
	Path& Append(const Path& rhs);
	/** Add a path at the end of the first path */
	template<typename T>
	__forceinline Path& Append(const T& rhs) { return (Append(Path(rhs))); }

	/** Add a path at the start of the first path */
	Path& Prepend(const Path& rhs);
	/** Add a path at the start of the first path */
	template<typename T>
	__forceinline Path& Prepend(const T& rhs) { return (Prepend(Path(rhs))); }

	/** Remove segments that are between the start and end index (included) */
	Path& TrimSegments(size_type start, size_type end);
	/** Remove the target of the path (e.g. "A/B/Image.png" => "A/B" */
	Path& TrimTarget();

	/** Add a segment to the end of the path */
	__forceinline Path& AppendSegment(const std::string_view segment) { return (InsertSegment(m_Path.size(), segment)); }
	/** Add a segment to the start of the path */
	__forceinline Path& PrependSegment(const std::string_view segment) { return (InsertSegment(0, segment)); }
	/**
	 * Add a segment at a given position.
	 *
	 * @param segmentIndex, the index where the segment will be added
	 * @return himself
	 */
	Path& InsertSegment(size_type segmentIndex, const std::string_view segment);

	/** Find the position of a segment that is equal to the given string */
	size_type FindSegment(const std::string_view segment) const;
	/** Find the position of a segment that is equal to the given string */
	__forceinline size_type FindSegment(const char* segment) const { return (FindSegment(std::string_view(segment))); }
	/** Find the position of a segment that is equal to the given string */
	template<typename T>
	__forceinline size_type FindSegment(const T& segment) const { return (FindSegment(std::string_view(segment))); }

	/** Find the position of a segment that is equal to the given string, but starting from the end */
	size_type RightFindSegment(const std::string_view segment) const;
	/** Find the position of a segment that is equal to the given string, but starting from the end */
	__forceinline size_type RightFindSegment(const char* segment) const { return (RightFindSegment(std::string_view(segment))); }
	/** Find the position of a segment that is equal to the given string, but starting from the end */
	template<typename T>
	__forceinline size_type RightFindSegment(const T& segment) const { return (RightFindSegment(std::string_view(segment))); }

public:
	/**
	 * Get the path has an std::string.
	 * @note by default this function will use the operating system separator.
	 * if you don't want this behavior use @see GetPath(char separator) const
	 *
	 * \return the path as an std::string
	 */
	__forceinline std::string GetPath() const { return GetPath(Path::PlatformSeparator); }
	/** Get the path has an std::string using the separator passed as parameter */
	std::string GetPath(const char separator) const;

	/** Get the file targeted by the path (e.g. "Image.png") or "" if no target */
	const std::string_view GetFileTarget() const;
	/** Get the file extension of the targeted file (e.g. "png") or "" if no target */
	const std::string_view GetExtension() const;
	/** Get the name of the file target by the path (e.g. "Image") or "" if no target */
	const std::string_view GetFileName() const;

	/** Return how many segments the path has */
	__forceinline size_type GetSegmentCount() const { return (static_cast<size_type>(m_Path.size() )); }
	/** Return whether or not the Path is empty */
	__forceinline bool IsEmpty() const { return (m_Path.empty()); }
#pragma endregion

private:
	void SplitOntoSegment(const std::string_view path);

private:
	/**
	 * The path stored as a vector of string, each string is a segment of the full path
	 * e.g: "C:/Users/MyUser/Documents/Image.png" will be stored as ["C:", "Users", "MyUser", "Documents", "Image.png"]
	 */
	std::vector<std::string> m_Path;

#pragma region Static - API
public:
	/** return the root directory of the engine */
	static Path GetEngineRootDirectoryPath();
	/** return the path where all the module DLL are stored */
	static Path GetModuleDirectoryPath();
	/** return the path where all the log files are stored */
	static Path GetLogDirectoryPath();
	/** return the path where all the generated data file are stored */
	static Path GetSavedDirectoryPath();

	/** return whether or not the char is separator of some sort or not */
	__forceinline static bool IsSeparator(const char c) { return (c == DefaultSeparator || c == WindowsSeprator); }

public:
	/* The default separator used by the engine to separate the path segments */
	static constexpr char DefaultSeparator = '/';
	/* The separator windows use to separate the path segments */
	static constexpr char WindowsSeprator = '\\';
#ifdef PLATFORM_WINDOWS
	/* The separator that the platform use to separate the path segments */
	static constexpr char PlatformSeparator = WindowsSeprator;
#else
	/* The separator that the platform use to separate the path segments */
	static constexpr char PlatformSeparator = DefaultSeparator;
#endif // PLATFORM_WINDOWS
#pragma endregion

private:
	struct DataCache
	{
		Path* EngineRootDirectory;
		Path* ModuleDirectory;
	};
	static DataCache s_Data;
};
