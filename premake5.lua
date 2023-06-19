include "./vendor/Premake5/GlobalVariable.lua"
include "./vendor/Premake5/Extended/workspace_files.lua"

workspace "OpenVoxel"
	architecture "x64"
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
	include "vendor/Premake5"
	include "Source/ThirdParty/imgui"
	include "Source/ThirdParty/GLFW"
group ""

include "Source/premake5.lua" -- OpenVoxel

group "Programs"
	include "Source/Programs/CrashHandler/premake5.lua" -- Crash Handler
group ""
