include "Constants.lua"

function UseDefaultDefines()
	-- Config specific
	filter "configurations:*Debug"
		defines "OV_DEBUG"
	filter "configurations:Editor*"
		defines "WITH_EDITOR"

	-- System specific
	filter "system:windows"
		defines "PLATFORM_WINDOWS"

	filter "system:linux"
		defines "PLATFORM_LINUX"

	filter "system:macosx"
		defines "PLATFORM_MAC"
	filter {}
end

-------------------------------------------------------------------------------
-- PROJECTS -------------------------------------------------------------------
-------------------------------------------------------------------------------

include "Extended/FastUpToDateCheck.lua" -- Allow the project to disable fast up to date check

local ProjectObjectOutput = INTERMEDIATE_OUTPUT_PATH .. "/%{prj.name}"

function UseProjectDefaultConfig()
	UseDefaultDefines()

	kind "ConsoleApp"

	-- Disable Fast Up To Date check, to allow the project to update the dlls after building modules
	FastUpToDateCheck(false)

	language "C++"
	cppdialect "C++20"

	staticruntime "off"
	systemversion "latest"

	location(PROJECT_FILE_OUTPUT_PATH) -- Where does the config file for the project will be generated
	targetdir(PROJECT_OUTPUT_DIR) -- Where the project will be generated
	objdir(ProjectObjectOutput) -- Where the project object files will be generated

	-- Config specific
	filter "configurations:*Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:*Release"
		runtime "Release"
		optimize "on"
		symbols "off"
	filter {}
end

-------------------------------------------------------------------------------
-- MODULES --------------------------------------------------------------------
-------------------------------------------------------------------------------

local ModuleObjectOutput = INTERMEDIATE_OUTPUT_PATH .. "/Modules"

function UseModuleDefaultConfig()
	UseDefaultDefines()

	kind "SharedLib" -- Dll

	language "C++"
	cppdialect "C++20"

	location(PROJECT_FILE_OUTPUT_PATH) -- Where does the config file for the project will be generated
	targetdir(MODULE_OUTPUT_PATH) -- Where the project will be generated
	objdir(ModuleObjectOutput) -- Where the project object files will be generated

	-- Config specific
	filter "configurations:*Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:*Release"
		runtime "Release"
		optimize "on"
		symbols "off"
	filter {}

	-- Compile with /MDd
	staticruntime "off"
	systemversion "latest"
end
