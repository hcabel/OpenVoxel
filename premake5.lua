include "Source/Programs/BuildTools/GeneratePremakeFile.lua"

include "./vendor/Premake5/GlobalVariable.lua"
include "./vendor/Premake5/Extended/workspace_files.lua" -- Allow files to be added to the workspace

workspace "OpenVoxel"
	architecture "x86_64"
	configurations
	{
		"Runtime - Debug",
		"Runtime - Release",
		"Editor - Debug",
		"Editor - Release",
		-- TODO: Add dist config for release builds
		-- "DIST"
	}

	workspace_files
	{
		".editorconfig",
		".gitignore",
		".gitmodules",
		"imgui.ini",
	}

	startproject "OpenVoxel"

group "Dependencies"
	include "Source/ThirdParty/imgui"
	include "Source/ThirdParty/GLFW"
group ""

group "Engine"
	include "Source/premake5.lua" -- OpenVoxel

	-- Include all modules
	group "Engine/Modules/Runtime"
		GenerateModuleProject({
			"Core",
			"Renderer",
			"Engine",
		})
	group "Engine/Modules/Editor"
		GenerateModuleProject({
			"Editor",
			"UI",
		})
	group ""
group ""

group "Programs"
	include "Source/Programs/CrashHandler/premake5.lua" -- Crash Handler
group ""

include "vendor/Premake5"

