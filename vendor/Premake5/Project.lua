include "GlobalVariable.lua"
include "Shared.lua"

include "Extended/FastUpToDateCheck.lua" -- Allow the project to disable fast up to date check

ProjectTargetOutput = BuildOutput .. "/%{prj.name}"
ProjectObjectOutput = IntermediateOutput .. "/%{prj.name}"

function UseProjectDefaultConfig()
	UseDefines()

	-- Disable Fast Up To Date check, to allow the project to update the dlls after building modules
	FastUpToDateCheck(false)

	language "C++"
	cppdialect "C++20"

	location(ProjectFileLocationOutput) -- Where does the config file for the project will be generated
	objdir(ProjectObjectOutput) -- Where the project object files will be generated
	targetdir(ProjectTargetOutput) -- Where the project will be generated

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

