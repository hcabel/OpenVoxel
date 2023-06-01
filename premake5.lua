
VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

rootDir = os.getcwd()

buildDir = rootDir .. "/build"
intermediateDir = rootDir .. "/intermediate"

buildOutput = buildDir .. "/" .. outputdir
intermediateOutput = intermediateDir .. "/" .. outputdir

projectFileLocation = rootDir .. "/intermediate/ProjectFile"

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

	startproject "OpenVoxel"

group "Dependencies"
	include "Source/ThirdParty/imgui"
	include "Source/ThirdParty/GLFW"
group ""

include "Source/premake5.lua" -- OpenVoxel

group "Programs"
	include "Source/Programs/CrashHandler/premake5.lua" -- Crash Handler
group ""
