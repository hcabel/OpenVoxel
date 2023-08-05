include "Source/Programs/BuildTools/GeneratePremakeFile.lua"
include "./Source/Programs/BuildTools/PremakeExtended/workspace_files.lua" -- Allow files to be added to the workspace

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

	startproject "Launch"

group "Dependencies"
	include "Source/ThirdParty/imgui"
	include "Source/ThirdParty/GLFW"
group ""

group "Engine"
	GenerateModuleProject({
		"Core",
		"Renderer",
		"Engine",
		"Launch"
	})
	group "Engine/Editor"
		GenerateModuleProject({
			"Editor",
			"UI",
		})
	group ""
group ""

group "Programs"
group ""

-- The premake project allow you to generate the solution file when you build the project from the IDE
project "Premake"
	kind "Utility"

	UseProjectDefaultConfig()

	postbuildmessage ("Generating solution...")
	postbuildcommands {
		-- @TODO: investigate why the line bellow is not working. for some reason windows does not find the file :/
		-- '"' .. ROOT_DIR_PATH .. 'GenerateSolution.bat"'
		'"' .. ROOT_DIR_PATH .. 'vendor/bin/premake5.exe" --file="' .. ROOT_DIR_PATH .. 'premake5.lua" vs2022'
	}
