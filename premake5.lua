
VULKAN_SDK = os.getenv("VULKAN_SDK")

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

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Source/ThirdParty/imgui"
	include "Source/ThirdParty/GLFW"
group ""

include "./premake_OpenVoxel.lua"

