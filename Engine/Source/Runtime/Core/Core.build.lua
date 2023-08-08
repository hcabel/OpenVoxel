﻿
function CoreModule(config)
	Core = {} -- Set module object

	Core.Kind = "Module" -- Will be build as a dll

	-- Source file of the module
	Core.Files = {
		"**.h",
		"**.cpp",
		"**.lua",
	}

	-- Include directories of the module
	--
	-- Module that include this module will add those Public_IncludeDirs to their include directories (automatically)
	-- e.g. Instead of "Public/HAL/Window.h" other modules will be able to do "Window.h"
	Core.Public_IncludeDirs = {
		"Public",
		-- "Public/HAL"
	}
	-- The Private_IncludeDirs are shortcuts for this module only
	Core.Private_IncludeDirs = {
		"Private",
	}

	-- Libraries to link against
	--
	-- List of the module that required to be linked against
	Core.ModuleDependency = {}
	-- List of the third party libraries that required to be linked against
	Core.ThirdPartyDependency = {
		"GLFW",
		"glm",
	}

	-- Defines
	--
	-- Extra Defines that can be used in this module
	Core.Defines = {
		"OV_BUILD_CORE_DLL"
	}

	return Core;
end

return CoreModule;