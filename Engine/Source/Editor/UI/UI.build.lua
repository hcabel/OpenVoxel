
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
	}
	UI.ThirdPartyDependency = {
		"GLFW",
		"Vulkan",
		"ImGui",
	}

	UI.Defines = {
		"OV_BUILD_UI_DLL"
	}

	return UI
end

return UIModule