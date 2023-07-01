include "GlobalVariable.lua"

function UseDefines()

	-- Config specific
	filter "configurations:*Debug"
		defines "OV_DEBUG"
	filter "configurations:Editor*"
		defines "WITH_EDITOR"

	-- System specific
	filter "system:windows"
		defines "PLATFORM_WINDOWS"

	filter "system:linux"
		defines "PLATFORM_LINUX"

	filter "system:macosx"
		defines "PLATFORM_MAC"
	filter {}
end
