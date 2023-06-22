#pragma once

#include "Core_API.h"

#include <string>

class CORE_API Path
{
public:
	/** The the root directory of the engine */
	static std::string GetEngineRootDirectoryPath();
};
