
include "../GlobalValues.lua"
include "../FileUtils.lua"
include "../DebugUtils.lua"

include "Utils.lua"

local function BuildFileLoadingError(buildFilePath, errorMessage)
	error("Failed to load build file: '" .. (buildFilePath or "") .. "'. " .. (errorMessage or ""))
end

local function FillTheGaps(buildData, emptyBuildData)
	-- Loop over each field of the empty build data and if the field does not exist in the build data, copy it
	for key, value in pairs(emptyBuildData) do
		if buildData[key] == nil then
			buildData[key] = value
		end
	end
end

local function HandleStaticData(buildData, newBuildData)
	-- Those are the field that the build file need to have but are not configuration specific
	local staticDataField = {
		{
			Name = "Kind",
			DefaultValue = "Module"
		}
	}

	for _, field in ipairs(staticDataField) do
		if buildData[field.Name] == nil then
			buildData[field.Name] = (newBuildData[field.Name] or field.DefaultValue)
		end
		newBuildData[field.Name] = nil
	end
end

-- this function take two object and remove the common data from the config data
local function RemoveCommonData(commonBuildData, configBuildData)
	local cleanedData = {}

	for key, value in pairs(configBuildData) do
		if type(value) == "table" then
			cleanedData[key] = RemoveCommonData(commonBuildData[key] or {}, value)
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
		local newBuildData = GetBuildDataFunc(config)

		-- Add empty data to avoid error when we try to access a field that does not exist
		FillTheGaps(newBuildData, CreateEmptyBuildData())
		-- Some data are "Static" they're not configuration specific, so we move them to the root
		HandleStaticData(buildData, newBuildData)
		-- Remove data that are already in the common data or are static
		buildData[configName]  = RemoveCommonData(buildData["Common"], newBuildData)

	end, buildData)


	return buildData
end

local function ResolveBuildDataPaths(buildFileData)
	-- list of all the build data fields that contain paths
	local pathFields = {
		"Public_IncludeDirs",
		"Private_IncludeDirs"
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