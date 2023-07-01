include "GlobalVariable.lua"
include "Shared.lua"

ModuleTargetOutput = BuildOutput .. "/Modules"
ModuleObjectOutput = IntermediateOutput .. "/Modules"

function UseModuleDefaultConfig()
	UseDefines()

	kind "SharedLib" -- Dll

	language "C++"
	cppdialect "C++20"

	location(ProjectFileLocationOutput) -- Where does the config file for the project will be generated
	objdir(ModuleObjectOutput) -- Where the project object files will be generated
	targetdir(ModuleTargetOutput) -- Where the project will be generated

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
