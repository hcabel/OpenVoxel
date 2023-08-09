
function EditorModule(config)
	local Editor = {}

	Editor.Kind = "Module"

	Editor.Public_IncludeDirs = {
		"Public",
	}
	Editor.Private_IncludeDirs = {
		"Private",
	}

	Editor.ModuleDependency = {
		"Core",
		"Engine",
		"Renderer",
		"UI",
	}
	Editor.ThirdPartyDependency = {}

	Editor.Defines = {
		"OV_BUILD_EDITOR_DLL"
	}

	return Editor
end

return EditorModule