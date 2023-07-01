include "GlobalVariable.lua"

project "Premake"
	kind "Utility"
	
	location (projectFileLocation)

	files
	{
		RootDir .. "/vendor/Premake5/**.lua",
		RootDir .. "/**premake5.lua"
	}

	postbuildmessage ("Generating solution...")
	postbuildcommands
	{
		-- @TODO: investigate why the line bellow is not working. for some reason windows does not find the file :/
		-- RootDir .. "GenerateSolution.bat"
		RootDir .. '/vendor/bin/premake5.exe --file="' .. RootDir .. '/premake5.lua" vs2022'
	}
