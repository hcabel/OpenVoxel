
function CoreModule(config)
	Core = {} -- Set module object

	Core.Kind = "Module" -- Will be build as a dll (optional, by default it's "Module")

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
	-- @note Modules have 1 define by default: OV_BUILD_[UpperCaseModuleName]_DLL (For the module API)
	Core.Defines = {}

	return Core;
end

return CoreModule;