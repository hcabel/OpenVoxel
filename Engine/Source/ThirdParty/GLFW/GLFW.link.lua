
-- Libraries are build using premake5.lua
-- but here you can add data that will help other module to link with this one
-- Like Public_IncludeDirs
function GLFWModule(config)
	local GLFW = {}

	GLFW.LinkName = "GLFW"

	GLFW.IncludeDirs = {
		"include",
	}

	return GLFW
end

return GLFWModule