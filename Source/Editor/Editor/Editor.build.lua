
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

		-- REMOVE
		"../../ThirdParty/glm",
		"../../ThirdParty/GLFW/include",
		"%{VULKAN_SDK}/Include",
	}

	Editor.ModulesDependencies = {
		"Core",
		"Engine",
		"Renderer",
		"UI",
	}
	Editor.LibrariesDependencies = {
		"GLFW",
		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
	}

	Editor.Defines = {
		"OV_BUILD_EDITOR_DLL"
	}

	return Editor
end

return EditorModule