require "vscode"

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
	include "Source/ThirdParty/imgui"
	include "Source/ThirdParty/GLFW"
group ""

group "Engine"
	include "Source/premake5.lua" -- OpenVoxel

	-- Include all modules
	group "Engine/Modules"
		for _, moduleName in ipairs(OV_RuntimeModules) do
			include ("Source/Runtime/" .. moduleName .. "/premake5.lua")
		end
	group ""
group ""

group "Programs"
	include "Source/Programs/CrashHandler/premake5.lua" -- Crash Handler
group ""

include "vendor/Premake5"

