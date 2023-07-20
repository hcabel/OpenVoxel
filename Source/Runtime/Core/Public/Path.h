#pragma once

#include "Core_API.h"

#include <string>

class CORE_API Path
{
public:
	/** return The the root directory of the engine */
	static std::string GetEngineRootDirectoryPath();
	/** return the path where all the module DLL are stored */
	static std::string GetModuleDirectoryPath();

private:
	struct DataCache
	{
		std::string EngineRootDirectory;
		std::string ModuleDirectory;
	};
	static DataCache s_Data;
};
