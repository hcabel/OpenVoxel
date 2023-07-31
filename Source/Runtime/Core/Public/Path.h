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

	/** return the path where all the log files are stored */
	static std::string GetLogDirectoryPath();
	/** return the path where all the generated data file are stored */
	static std::string GetSavedDirectoryPath();

private:
	struct DataCache
	{
		std::string EngineRootDirectory;
		std::string ModuleDirectory;
		std::string SavedDirectory;
	};
	static DataCache s_Data;
};
