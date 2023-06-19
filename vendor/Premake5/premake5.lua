include "GlobalVariable.lua"

project "Premake"
	kind "Utility"
	
	location (projectFileLocation)

	files
	{
		rootDir .. "vendor/Premake5/**.lua",
		rootDir .. "**premake5.lua"
	}

	postbuildmessage ("Generating solution...")
	postbuildcommands
	{
		-- @TODO: investigate why the line bellow is not working. for some reason windows does not find the file :/
		-- rootDir .. "GenerateSolution.bat"
		rootDir .. 'vendor/bin/premake5.exe --file="' .. rootDir .. 'premake5.lua" vs2022'
	}
