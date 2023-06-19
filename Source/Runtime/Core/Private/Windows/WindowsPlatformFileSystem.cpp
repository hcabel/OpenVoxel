#include "Windows/WindowsPlatformFileSystem.h"

#include <Windows.h>

std::string WindowsPlatformFileSystem::MakeEngineRootDirectoryPath()
{
	// Get binairy path
	char buffer[MAX_PATH];
	if (GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0)
		return ("");
	std::string binairyPath = std::string(buffer);

	// Remove everything after "build/"
	size_t posOfBuildFolder = binairyPath.rfind("build\\");
	if (posOfBuildFolder != std::string::npos)
		binairyPath = binairyPath.erase(posOfBuildFolder); 
	return (binairyPath);
}
