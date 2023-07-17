include "../../../vendor/Premake5/Utils.lua"

project "Editor"
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

		-- Modules
		"../../../Source/Runtime/Core/Public",
		"../../../Source/Runtime/Engine/Public",
	}

	links
	{
		-- ThirdParty
		
		-- Modules
		"Core",
		"Engine",
	}

	defines "OV_BUILD_EDITOR_DLL"
