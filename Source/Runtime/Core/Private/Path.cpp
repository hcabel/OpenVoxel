#include "Path.h"

#include "Singleton.h"
#include "HAL/PlatformFileSystem.h"

Path::DataCache Path::s_Data;

/**
 * Create the content of a function that will get a path from the cache
 * if the cache is empty, it will call the getter function in the PlatformFileSystem to get the path.
 * @param VariableName the name of the variable that will be used to store the path in the cache
 * @note The getter function must be named Make[VariableName]Path in the PlatformFileSystem class
 */
#define PATH_GETTER_CACHED(VariableName) \
	/* Get the VariableName from in cache */ \
	std::string& VariableName = s_Data.VariableName; \
	if (VariableName.empty()) \
	{ \
		/* if cache is empty, get the value using the getter function */ \
		VariableName = PlatformFileSystem::Make##VariableName##Path(); \
	} \
	return (VariableName); \

std::string Path::GetEngineRootDirectoryPath()
{
	PATH_GETTER_CACHED(EngineRootDirectory);
}

std::string Path::GetModuleDirectoryPath()
{
	PATH_GETTER_CACHED(ModuleDirectory);
}

std::string Path::GetLogDirectoryPath()
{
	return (GetSavedDirectoryPath() + "logs\\");
}

std::string Path::GetSavedDirectoryPath()
{
	return (GetEngineRootDirectoryPath() + "saved\\");
}

#undef PATH_GETTER_CACHED
