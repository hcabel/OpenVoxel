#include "Path.h"

#include "Singleton.h"
#include "HAL/PlatformFileSystem.h"
#include "EngineStaticData.h"

std::string Path::GetEngineRootDirectoryPath()
{
	// Get the root directory from the singleton
	std::string& rootDirectory = Singleton<EngineStaticData>::Get().EngineRootDirectory;
	if (rootDirectory.empty())
	{
		// if does not exist (first call), create it
		rootDirectory = PlatformFileSystem::MakeEngineRootDirectoryPath();
	}
	return (rootDirectory);
}

