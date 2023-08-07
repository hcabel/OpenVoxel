
function OpenVoxelApplication(config)
	local OpenVoxel = {}

	OpenVoxel.Kind = "Application"

	OpenVoxel.Files = {
		"**.h",
		"**.cpp"
	}

	OpenVoxel.Public_IncludeDirs = {}
	OpenVoxel.Private_IncludeDirs = {
		"Private",
	}

	OpenVoxel.ModulesDependencies = {
		"Core",
		"Engine",
		"Renderer",
	}
	OpenVoxel.LibrariesDependencies = {
		"Vulkan",
		"GLFW",
		"glm",
	}

	-- check whether or not the configuration START with "Editor"
	if config.configuration:find("Editor") then
		local EditorModules = {
			"Editor",
			"UI",
		}
		for _, EditorModule in ipairs(EditorModules) do
			table.insert(OpenVoxel.ModulesDependencies, EditorModule)
		end

		local EditorLibraries = {
			"ImGui",
		}
		for _, EditorLibrary in ipairs(EditorLibraries) do
			table.insert(OpenVoxel.LibrariesDependencies, EditorLibrary)
		end
	end

	return OpenVoxel
end

return OpenVoxelApplication