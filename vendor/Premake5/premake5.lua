project "Premake"
	kind "Utility"
	
	location (projectFileLocation)

	targetdir (buildOutput .. "/%{prj.name}")
	objdir (intermediateOutput .. "/%{prj.name}")

	files
	{
		"**.lua",
		"%{wks.location}/**premake5.lua"
	}

	postbuildcommands
	{
		"%{wks.location}/GenerateSolution.bat"
	}