
project "CrashHandler"
	kind "ConsoleApp"
	language "C++"
	location (projectFileLocation)

	targetdir (buildOutput .. "/%{prj.name}")
	objdir (intermediateOutput .. "/%{prj.name}")

