include "../../../vendor/Premake5/Utils.lua"

project "UI"
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
		"../../ThirdParty/glm",
		"../../ThirdParty/GLFW/include",
		"%{VULKAN_SDK}/Include",
		"../../ThirdParty/imgui",

		-- Modules
		"../../../Source/Runtime/Core/Public",
		"../../../Source/Runtime/Renderer/Public",
	}

	links
	{
		-- ThirdParty
		"GLFW",
		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
		"ImGui",
		
		-- Modules
		"Core",
		"Renderer",
	}

	defines "OV_BUILD_UI_DLL"
