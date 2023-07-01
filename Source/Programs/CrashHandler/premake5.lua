include "../../../vendor/Premake5/Utils.lua"

project "CrashHandler"
	kind "ConsoleApp"

	UseProjectDefaultConfig()

	files
	{
		"Private/**.cpp",
		"Private/**.h"
	}
