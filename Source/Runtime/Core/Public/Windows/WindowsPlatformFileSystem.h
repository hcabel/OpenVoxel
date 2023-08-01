#pragma once

#include "Core_API.h"

class Path;

/**
 * A helper class to interact with Windows file system
 */
class CORE_API WindowsPlatformFileSystem
{
public:
	/** Find Engine root directory Path */
	static Path MakeEngineRootDirectoryPath();
	/** Find the path where all the module are stored */
	static Path MakeModuleDirectoryPath();
};

typedef WindowsPlatformFileSystem PlatformFileSystem;
