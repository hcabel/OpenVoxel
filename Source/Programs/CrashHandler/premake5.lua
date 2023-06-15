
project "CrashHandler"
	kind "ConsoleApp"
	language "C++"
	location (projectFileLocation)

	targetdir (buildOutput .. "/%{prj.name}")
	objdir (intermediateOutput .. "/%{prj.name}")

	files
	{
		"premake5.lua",
		"Private/**.cpp",
		"Private/**.h"
	}
