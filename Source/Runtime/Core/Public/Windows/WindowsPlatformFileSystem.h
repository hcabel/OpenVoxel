#pragma once

#include "Core_API.h"

#include <string>

/**
 * A helper class to interact with Windows file system
 */
class CORE_API WindowsPlatformFileSystem
{
public:
	/** Find Engire root directory Path */
	static std::string MakeEngineRootDirectoryPath();
};

typedef WindowsPlatformFileSystem PlatformFileSystem;
