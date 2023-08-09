
include "../GlobalValues.lua"
include "../FileUtils.lua"
include "../DebugUtils.lua"

include "Utils.lua"

local function BuildFileLoadingError(buildFilePath, errorMessage)
	error("Failed to load build file: '" .. (buildFilePath or "") .. "'. " .. (errorMessage or ""))
end

local function MoveNonConfigDataToTheRoot(buildData)
	-- Those are the field that the build file need to have but are not configuration specific
	local nonConfigDataField = {
		"Kind"
	}

	for _, field in ipairs(nonConfigDataField) do
		buildData[field] = buildData["Common"][field]
		buildData["Common"][field] = nil
	end
end

-- this function take two object and remove the common data from the config data
local function RemoveCommonData(configBuildData, commonBuildData)
	local cleanedData = {}

	for key, value in pairs(configBuildData) do
		if type(value) == "table" then
			cleanedData[key] = RemoveCommonData(value, commonBuildData[key])
		else
			if value ~= commonBuildData[key] then
				-- if commonBuildData is a table and the key is a number use the table.insert function (otherwise we would have a hole in the array)
				if type(commonBuildData) == "table" and type(key) == "number" then
					table.insert(cleanedData, value)
				else
					cleanedData[key] = value
				end
			end
		end
	end

	return cleanedData
end

local function CleanConfigData(buildData, configBuildData)

	-- Remove data that are already in the common data or are static
	local cleanedData = RemoveCommonData(configBuildData, buildData["Common"])

	-- Remove data that are static
	local staticDataField = {
		"Kind"
	}

	for _, field in ipairs(staticDataField) do
		cleanedData[field] = nil
	end

	return cleanedData
end

local function LoadBuildFile(buildFilePath)
	-- Load the module's build file
	local GetBuildDataFunc = dofile(buildFilePath)
	if GetBuildDataFunc == nil then
		BuildFileLoadingError(buildFilePath, "File does not return the get build data function.")
	end

	local buildData = {} -- Result of this function
	local config = {
		configuration = "",
		architecture = WKS.architecture, -- TODO: Will probably have to loop over all the architecture too
		platform = WKS.platform,
	}

	DoXForEveryConfig(function (configData, configName)
		if configName ~= "Common" then
			config.configuration = configName
		end

		-- We run the function and store the data for the current configuration
		local configBuildData = GetBuildDataFunc(config)

		if configName == "Common" then
			buildData[configName] = configBuildData
			MoveNonConfigDataToTheRoot(buildData)
		else
			-- Remove data that are already in the common data or are static
			buildData[configName] = CleanConfigData(buildData, configBuildData)
		end
	end, buildData)

	return buildData
end

local function ResolveBuildDataPaths(buildFileData)
	-- list of all the build data fields that contain paths
	local pathFields = {
		"Public_IncludeDirs",
		"Private_IncludeDirs",
		"Files",
	}

	-- Loop over each field that contain path for every configuration and common data
	for _, field in ipairs(pathFields) do
		DoXForEveryConfig(function(data)
			ResolveRelativePaths(data[field], buildFileData.RootDirectory)
		end, buildFileData)
	end
end

local function GetPathAndEngineScope(moduleName)

	-- Get file Path and engine scope
	-- We firstly assume that's a runtime module
	local returnBundle = {
		EngineScope = "Runtime",
		Path = "Source/Runtime/" .. moduleName .. "/" .. moduleName .. ".build.lua"
	}

	if FileExists(returnBundle.Path) == false then
		-- if not, we assume that's an editor module
		returnBundle = {
			EngineScope = "Editor",
			Path = "Source/Editor/" .. moduleName .. "/" .. moduleName .. ".build.lua"
		}

		-- Is not an editor module either, we throw an error
		if FileExists(returnBundle.Path) == false then
			BuildFileLoadingError("'Source/[Runtime|Editor]/" .. moduleName .. "/" .. moduleName .. ".build.lua", "Build file not found!")
		end
	end

	return returnBundle
end

function LoadModuleFromDisk(moduleName)

	local bundle = GetPathAndEngineScope(moduleName)

	local moduleFilePath = bundle.Path
	local engineScope = bundle.EngineScope

	local buildFileData = LoadBuildFile(moduleFilePath, engineScope)

	-- Add static data (those data does not affect the build process, it's just handy to have)
	buildFileData.Name = moduleName
	buildFileData.EngineScope = engineScope
	buildFileData.RootDirectory = ROOT_DIR_PATH .. "Source/" .. engineScope .. "/" .. moduleName .. "/"

	-- If the module is an editor module, we move all the common data to each Editor configuration
	if buildFileData.Kind == "Module" and buildFileData.EngineScope == "Editor" then

		DoXForEveryConfig(function(data, configuration)

			-- Loop over source and if the entry does not exist in destination, copy it
			local function RecursiveCopyTable(source, destination)
				for key, value in pairs(source) do
					if type(value) == "table" then
						if destination[key] == nil then
							destination[key] = {}
						end
						RecursiveCopyTable(value, destination[key])
					else
						if destination[key] == nil then
							destination[key] = value
						end
					end
				end
			end

			if configuration:find("Editor") then
				data = RecursiveCopyTable(buildFileData["Common"], data)
			elseif configuration ~= "Common" then
				buildFileData[configuration] = CreateEmptyBuildData() -- We don't need non Editor configuration
			end
		end, buildFileData)
		buildFileData["Common"] = CreateEmptyBuildData() -- Remove common new that we copied to each Editor configuration
	end

	-- Resolve all path to absolute path
	ResolveBuildDataPaths(buildFileData)

	print("'" .. moduleName .. "' has been successfully loaded from disk")

	return buildFileData
end