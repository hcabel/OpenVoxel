include "../../../vendor/Premake5/Utils.lua"

project "Engine"
	UseModuleDefaultConfig()

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
		"%{VULKAN_SDK}/Include",
		"../../ThirdParty/GLFW/include",
		"../../ThirdParty/glm",

		-- Modules
		"../Core/Public",
		"../Renderer/Public",
	}

	links
	{
		-- ThirdParty
		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
		"GLFW",
		
		-- Modules
		"Core",
		"Renderer",
	}

	defines "OV_BUILD_ENGINE_DLL"
