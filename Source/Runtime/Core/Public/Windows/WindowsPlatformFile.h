#pragma once

#include "Core_API.h"

#include <string_view>
#include <iostream>
#include <fstream>
#include <memory>
#include <filesystem>

/**
 * A class that provides file I/O functionality for the current platform.
 * 
 * WINDOW ONLY!
 */
class CORE_API WindowsPlatformFile
{
private:
	WindowsPlatformFile() = default;
	inline static std::shared_ptr<WindowsPlatformFile> CreateInstance() { return std::shared_ptr<WindowsPlatformFile>(new WindowsPlatformFile()); }

public:
	/** Use PlatformFile::Copy to copy a file */
	WindowsPlatformFile(const WindowsPlatformFile& other) = delete;
	~WindowsPlatformFile();

#pragma region API
public:
	/** Close the file stream */
	inline void Close() { m_FileStream.close(); }
	/** Tell whether or not the file is open */
	inline bool IsOpen() const { return (m_FileStream.is_open()); }
	/**
	 * Read the whole file in one go.
	 *
	 * \return a string containing the whole file.
	 */
	std::string ReadAll();

	template<typename T>
	inline WindowsPlatformFile& operator<<(const T& value) {
		m_FileStream << value;
		return *this;
	}
	inline WindowsPlatformFile& operator<<(std::ostream& (*func)(std::ostream&)) {
		m_FileStream << func;
		return *this;
	}
#pragma endregion

#pragma region API - Static
public:
	/**
	 * Copy the platform file to the path destination.
	 * 
	 * \param src The source file to copy
	 * \param path The destination path to copy the file to. (without the file name and extension)
	 */
	static std::shared_ptr<WindowsPlatformFile> Copy(const WindowsPlatformFile& src, std::string_view path) { return (nullptr); } // TODO: implement this function
	/**
	 * Open file or create it if doesn't exist.
	 * \note All the parent directories are created automatically.
	 * 
	 * \param fullPath The full path to the file to create. (including the file name and extension)
	 * \return the handle of the new file.
	 */
	static std::shared_ptr<WindowsPlatformFile> Open(std::string_view fullPath, std::ios_base::openmode openmode);
	/**
	 * Create a directory at the specified path. (including all the parent directories)
	 * \note that when creating a file the parent directory are created automatically.
	 * 
	 * \param path The path to the directory to create.
	 * \return true if the directory was created, false otherwise.
	 */
	inline static bool CreateDirectory(std::string_view path) { return (std::filesystem::create_directories(path)); }
	inline static bool CreateDirectory(std::filesystem::path path) { return (std::filesystem::create_directories(path)); }
#pragma endregion

protected:
	std::string_view m_FullPath;
	std::fstream m_FileStream;

};

typedef WindowsPlatformFile PlatformFile;
