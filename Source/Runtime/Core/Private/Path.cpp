#include "Path.h"

#include "Singleton.h"
#include "HAL/PlatformFileSystem.h"

Path::DataCache Path::s_Data;

std::string Path::GetEngineRootDirectoryPath()
{
	// Get the root directory from in cache
	std::string& rootDirectory = s_Data.EngineRootDirectory;
	if (rootDirectory.empty())
	{
		// if cache is empty, create the root directory path
		rootDirectory = PlatformFileSystem::MakeEngineRootDirectoryPath();
	}
	return (rootDirectory);
}

