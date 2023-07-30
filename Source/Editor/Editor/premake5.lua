include "../../../vendor/Premake5/Utils.lua"

project "Editor"
	UseModuleDefaultConfig()

	filter "configurations:Editor*"
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

			-- Modules
			"../../../Source/Runtime/Core/Public",
			"../../../Source/Runtime/Engine/Public",
			"../../../Source/Runtime/Renderer/Public",
			"../../../Source/Editor/UI/Public",
		}

		links
		{
			-- ThirdParty
			"GLFW",
			"%{VULKAN_SDK}/Lib/vulkan-1.lib",

			-- Modules
			"Core",
			"Engine",
			"Renderer",
			"UI",
		}

		defines "OV_BUILD_EDITOR_DLL"
	filter {}
