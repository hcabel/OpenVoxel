#include "Windows/WindowsPlatformFile.h"
#include "Logging/LoggingMacros.h"
#include "CoreModule.h"

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
	m_FileStream.seekg(0, std::ios::end);
	std::streamsize fileSize = m_FileStream.tellg();
	m_FileStream.seekg(0, std::ios::beg);

	std::string content(fileSize, '\0');
	m_FileStream.read(&content[0], fileSize);

	return (content);
}
#pragma endregion

#pragma region API - Static
std::shared_ptr<WindowsPlatformFile> WindowsPlatformFile::Open(std::string_view fullPath, std::ios_base::openmode openmode)
{
	std::shared_ptr<WindowsPlatformFile> newFile = WindowsPlatformFile::CreateInstance();
	std::filesystem::path filePath = fullPath;

	if (!std::filesystem::exists(filePath.parent_path()))
		CreateDirectory(filePath.parent_path());

	newFile->m_FullPath = fullPath;
	newFile->m_FileStream = std::fstream(fullPath.data(), openmode);

	return (newFile);
}
#pragma endregion
