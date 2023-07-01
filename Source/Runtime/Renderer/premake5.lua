include "../../../vendor/Premake5/Utils.lua"

project "Renderer"
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

	defines "OV_BUILD_RENDERER_DLL"
