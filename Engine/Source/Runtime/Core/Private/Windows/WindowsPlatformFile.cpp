#include "Windows/WindowsPlatformFile.h"
#include "Logging/LoggingMacros.h"
#include "CoreGlobals.h"
#include "HAL/FileSystem.h"

#pragma region API
WindowsPlatformFile::~WindowsPlatformFile()
{
	if (IsOpen())
	{
		OV_LOG(CoreLog, Warning, "File was not closed before destruction!");
		Close();
	}
}

std::string WindowsPlatformFile::ReadAll()
{
	bool isOpen = IsOpen();

	// Open if not already opened
	if (isOpen == false)
		Open(std::ios_base::in);

	m_FileStream.seekg(0, std::ios::end);
	std::streamsize fileSize = m_FileStream.tellg();
	if (fileSize == -1)
	{
		OV_LOG(CoreLog, Error, "Failed to read file: '{:s}'", m_FullPath.data());
		return ("");
	}

	m_FileStream.seekg(0, std::ios::beg);

	std::string content(fileSize, '\0');
	m_FileStream.read(&content[0], fileSize);

	// Close the file if this function opened it
	if (isOpen == false)
		Close();
	return (content);
}
#pragma endregion

#pragma region API - Static
std::shared_ptr<WindowsPlatformFile> WindowsPlatformFile::Open(std::string_view fullPath, std::ios_base::openmode openmode)
{
	std::shared_ptr<WindowsPlatformFile> newFile = WindowsPlatformFile::CreateInstance();
	std::filesystem::path filePath = fullPath;

	if (std::filesystem::exists(filePath.parent_path()) == false)
	{
		// Create the directory if you open the file in write mode
		if (openmode & std::ios_base::out)
			CreateDirectory(filePath.parent_path());
		else
		{
			OV_LOG(CoreLog, Error, "Failed to open file: '{:s}'. Does not exist.", fullPath.data());
			newFile.reset();
			return (nullptr);
		}
	}

	newFile->m_FileStream.exceptions(newFile->m_FileStream.exceptions() | std::ios::failbit | std::ifstream::badbit);
	try {
		newFile->m_FileStream.open(fullPath.data(), openmode);
	}
	catch (std::ios_base::failure& e)
	{
		OV_LOG(CoreLog, Error, "Failed to open file: '{:s}'. {}", fullPath.data(), e.what());
		newFile.reset();
		return (nullptr);
	}

	if (newFile->IsOpen() == false)
	{
		OV_LOG(CoreLog, Error, "Failed to open file: {:s}", fullPath.data());
		newFile.reset();
		return (nullptr);
	}

	newFile->m_FullPath = fullPath;

	return (newFile);
}

std::unique_ptr<WindowsPlatformFile> WindowsPlatformFile::OpenUnique(std::string_view fullPath, std::ios_base::openmode openmode)
{
	std::unique_ptr<WindowsPlatformFile> newFile = WindowsPlatformFile::CreateUniqueInstance();
	std::filesystem::path filePath = fullPath;

	if (!std::filesystem::exists(filePath.parent_path()))
		CreateDirectory(filePath.parent_path());

	newFile->m_FullPath = fullPath;
	newFile->m_FileStream = std::fstream(fullPath.data(), openmode);

	if (newFile->IsOpen() == false)
	{
		OV_LOG(CoreLog, Error, "Failed to open file: {:s}", fullPath.data());
		newFile.reset();
		return (nullptr);
	}
	return (newFile);
}
#pragma endregion
