
include "../../../vendor/Premake5/GlobalVariable.lua"

project "Renderer"
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

		-- ThirdParty
		"../../ThirdParty/GLFW/include",
		"../../ThirdParty/glm",
		"%{VULKAN_SDK}/Include",

		-- Modules
		"../Core/Public",
	}

	links
	{
		-- ThirdParty
		"GLFW",
		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
		
		-- Modules
		"Core",
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

	-- /* DEFINES ************************************************************/

	defines "OV_BUILD_RENDERER_DLL"

	-- Config specific
	filter "configurations:Editor*"
		defines "WITH_EDITOR"
	filter "configurations:*Debug"
		defines "OV_DEBUG"

	-- System specific
	filter "system:windows"
		defines "PLATFORM_WINDOWS"
	filter "system:linux"
		defines "PLATFORM_LINUX"
	filter "system:macosx"
		defines "PLATFORM_MAC"
	filter {}

