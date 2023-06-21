
include "../../../vendor/Premake5/GlobalVariable.lua"

project "Core"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir (buildOutput .. "/OpenVoxel")
	objdir (intermediateOutput .. "/OpenVoxel")

	location (projectFileLocation)

	files
	{
		"**.h",
		"**.cpp"
	}

	includedirs
	{
		"Private",
		"Public",
		"../../ThirdParty/GLFW/include",
		"../../ThirdParty/glm",
	}

	links
	{
		"GLFW",
	}

	staticruntime "off"
	systemversion "latest"

	filter "configurations:*Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:*Release"
		runtime "Release"
		optimize "on"
		symbols "off"
	filter {}

	defines "PLATFORM_WINDOWS"
	defines "OV_BUILD_DLL"
