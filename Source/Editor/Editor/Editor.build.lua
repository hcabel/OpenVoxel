
function EditorModule(config)
	local Editor = {}

	Editor.Files = {
		"**.h",
		"**.cpp"
	}

	Editor.Public_IncludeDirs = {
		"Public",
	}
	Editor.Private_IncludeDirs = {
		"Private",
	}

	Editor.ModulesDependencies = {
		"Core",
		"Engine",
		"Renderer",
		"UI",
	}
	Editor.LibrariesDependencies = {
		"GLFW",
		"Vulkan",
		"glm",
	}

	Editor.Defines = {
		"OV_BUILD_EDITOR_DLL"
	}

	return Editor
end

return EditorModule