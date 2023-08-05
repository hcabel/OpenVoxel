
function OpenVoxelApplication(config)
	local OpenVoxel = {}

	OpenVoxel.Kind = "Application"

	OpenVoxel.Files = {
		"**.h",
		"**.cpp"
	}

	OpenVoxel.Public_IncludeDirs = {}
	OpenVoxel.Private_IncludeDirs = {
		"Runtime/Launch/Private",
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

	-- if string.starts(config.configurations, "Editor") then
	-- 	local EditorModules = {
	-- 		"Editor",
	-- 		"UI",
	-- 	}
	-- 	table.append(OpenVoxel.ModulesDependencies, EditorModules)

	-- 	local EditorLibraries = {
	-- 		"ImGui",
	-- 	}
	-- 	table.append(OpenVoxel.LibrariesDependencies, EditorLibraries)
	-- end

	return OpenVoxel
end

return OpenVoxelApplication