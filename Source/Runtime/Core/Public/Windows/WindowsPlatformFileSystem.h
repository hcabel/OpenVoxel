#pragma once

#include <string>

/**
 * A helper class to interact with Windows file system
 */
class WindowsPlatformFileSystem
{
public:
	/** Find Engire root directory Path */
	static std::string MakeEngineRootDirectoryPath();
};

typedef WindowsPlatformFileSystem PlatformFileSystem;
