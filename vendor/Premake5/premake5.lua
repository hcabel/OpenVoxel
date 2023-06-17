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

	postbuildmessage "Generating solution..."
	postbuildcommands
	{
		-- @TODO: investigate why the line bellow is not working. for some reason windows does not find the file :/
		-- "%{wks.location}GenerateSolution.bat"
		'%{wks.location}vendor/bin/premake5.exe --file="%{wks.location}premake5.lua" vs2022'
	}