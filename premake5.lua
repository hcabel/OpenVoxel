
VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

rootDir = os.getcwd()

buildDir = rootDir .. "/build"
intermediateDir = rootDir .. "/intermediate"

buildOutput = buildDir .. "/" .. outputdir
intermediateOutput = intermediateDir .. "/" .. outputdir

projectFileLocation = rootDir .. "/intermediate/ProjectFile"

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
		"premake5.lua"
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
