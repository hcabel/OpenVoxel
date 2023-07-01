include "../../../vendor/Premake5/Utils.lua"

project "Core"
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
	}

	links
	{
		-- ThirdParty
		"GLFW",
	}

	defines "OV_BUILD_CORE_DLL"
