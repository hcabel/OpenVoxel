include "Cache.lua"
include "Utils.lua"
include "Loader.lua"

local function GetAllModuleDependencyData(buildData)
	local modulesData = {}

	local function GetAllModuleDependencyData_Inner(currentBuildData, path)

		-- Check for circular dependencies
		for _, dependencyModuleName in ipairs(path) do
			if dependencyModuleName == currentBuildData.Name then
				error("Circular dependencies detected: '" .. table.concat(path, " -> ") .. "'")
			end
		end

		-- Update path
		local newPath = {}
		for _, dependencyModuleName in ipairs(path) do
			table.insert(newPath, dependencyModuleName)
		end
		table.insert(newPath, currentBuildData.Name)
		-- print ("=> " .. table.concat(newPath, " -> "));

		DoXForEveryConfig(function (buildData)
			for _, dependencyModuleName in ipairs(buildData.ModulesDependencies) do
				if modulesData[dependencyModuleName] == nil then
					modulesData[dependencyModuleName] = GetBuildData(dependencyModuleName)
					GetAllModuleDependencyData_Inner(modulesData[dependencyModuleName], newPath)
				end
			end
		end, currentBuildData)
	end

	GetAllModuleDependencyData_Inner(buildData, {})

	return modulesData
end

local function ResolveDependencies(buildData)

	local moduleDependenciesData = GetAllModuleDependencyData(buildData)

	DoXForEveryConfig(function (data, configuration)
		data.Resolved = CreateEmptyBuildData()

		for _, dependencyName in ipairs(data.ModulesDependencies) do
			local dependencyData = moduleDependenciesData[dependencyName]

			if dependencyData then
				-- Resolve public include dirs
				local dependencyIncludeDirs = GetFieldFromBuildData(dependencyData, "Public_IncludeDirs", configuration)
				for _, includeDir in ipairs(dependencyIncludeDirs) do
					table.insert(data.Resolved.Public_IncludeDirs, includeDir)
				end

			end
		end
	end, buildData)

	print ("Module '" .. buildData.Name .. "' has been resolved.")
	-- print (dump(buildData))
end

function GetBuildData(moduleName)

	-- Get data from cache
	local buildData = GetBuildDataCached(moduleName)

	-- If not loaded, load it
	if IsLoaded(buildData) == false then
		buildData = LoadModuleFromDisk(moduleName)
		buildData.LoadingSteps = MODULE_LOADED
		CacheBuildData(moduleName, buildData)
	end

	-- If not resolved, resolve it
	if IsResolved(buildData) == false then
		ResolveDependencies(buildData)
		buildData.LoadingSteps = MODULE_RESOLVED
		CacheBuildData(moduleName, buildData)
	end

	-- Return the loaded and resolved module
	return buildData
end