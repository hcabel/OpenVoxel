
function UIModule(config)
	local UI = {}

	UI.Kind = "Module"

	UI.Files = {
		"**.h",
		"**.cpp"
	}

	UI.Public_IncludeDirs = {
		"Public",
	}
	UI.Private_IncludeDirs = {
		"Private",
	}

	UI.ModulesDependencies = {
		"Core",
		"Renderer",
	}
	UI.LibrariesDependencies = {
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