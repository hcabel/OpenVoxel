
function UIModule(config)
	local UI = {}

	UI.Public_IncludeDirs = {
		"Public",
	}
	UI.Private_IncludeDirs = {
		"Private",
	}

	UI.ModuleDependency = {
		"Core",
		"Renderer",
		"Vulkan",
	}
	UI.ThirdPartyDependency = {
		"GLFW",
		"Vulkan",
		"ImGui",
		"stb_image"
	}

	return UI
end

return UIModule
