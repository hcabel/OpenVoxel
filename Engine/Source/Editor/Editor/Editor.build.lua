
function EditorModule(config)
	local Editor = {}

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

	return Editor
end

return EditorModule