
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

	OpenVoxel.ModuleDependency = {
		"Core",
		"Engine",
		"Renderer",
	}
	OpenVoxel.ThirdPartyDependency = {}

	-- check whether or not the configuration START with "Editor"
	if config.configuration:find("Editor") then

		local EditorModules = {
			"Editor",
			"UI",
		}
		for _, EditorModule in ipairs(EditorModules) do
			table.insert(OpenVoxel.ModuleDependency, EditorModule)
		end
	end

	return OpenVoxel
end

return OpenVoxelApplication