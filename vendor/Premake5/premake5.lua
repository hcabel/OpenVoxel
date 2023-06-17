project "Premake"
	kind "Utility"
	
	location (projectFileLocation)

	files
	{
		"**.lua",
		"%{wks.location}/**premake5.lua"
	}