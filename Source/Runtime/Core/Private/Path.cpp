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

std::string Path::GetModuleDirectoryPath()
{
	// Get the module directory from in cache
	std::string& moduleDirectory = s_Data.EngineRootDirectory;
	if (moduleDirectory.empty())
	{
		// if cache is empty, create the module directory path
		moduleDirectory = PlatformFileSystem::MakeModuleDirectoryPath();
	}
	return (moduleDirectory);

}
