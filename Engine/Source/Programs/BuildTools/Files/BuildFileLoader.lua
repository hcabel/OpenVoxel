include "../Constants.lua"
include "Utils.lua"
include "../Logger.lua"

local BuildFileCache = {}

local function BuildFileLoadingError(buildFilePath, errorMessage)
	Logger:Error("Failed to load build file: '" .. (buildFilePath or "") .. "'. " .. (errorMessage or ""))
end

local function LoadProjectBuildFileFromDisk(BuildFilePath)
	local projectName = GetPathTarget(BuildFilePath)
	Logger:Start("Loading build file \"" .. projectName .. "\" from disk...")

	if FileExists(BuildFilePath) == false then
		BuildFileLoadingError(BuildFilePath, "File does not exist.")
	end

	-- Load the module's build file, GetBuildDataFunc is what has been returned by the file
	local GetBuildDataFunc = dofile(BuildFilePath)
	if GetBuildDataFunc == nil then
		BuildFileLoadingError(BuildFilePath, "File does not return the get build data function.")
	end

	Logger:End(" Done")
	return GetBuildDataFunc
end

function LoadProjectBuildFile(BuildFilePath)
	if BuildFileCache[BuildFilePath] then
		return BuildFileCache[BuildFilePath]
	end

	local BuildFile = LoadProjectBuildFileFromDisk(BuildFilePath)
	BuildFileCache[BuildFilePath] = BuildFile

	return BuildFile
end

function GetProjectBuildFilePath(ProjectName, EngineScope)
	if not ProjectName or not EngineScope then
		BuildFileLoadingError("GetProjectBuildFilePath(ProjectName, EngineScope)", "ProjectName and EngineScope are required.")
	end
	return GetProjectRootPath(ProjectName, EngineScope) .. ProjectName .. ".build.lua"
end

function GetProjectRootPath(ProjectName, EngineScope)
	if not ProjectName or not EngineScope then
		BuildFileLoadingError("GetProjectRootPath(ProjectName, EngineScope)", "ProjectName and EngineScope are required.")
	end
	return ROOT_DIR_PATH .. "Source/" .. EngineScope .. "/" .. ProjectName .. "/"
end

function GetEngineScope(ProjectName)
	if not ProjectName then
		BuildFileLoadingError("GetEngineScope(ProjectName)", "ProjectName is required.")
	end

	local engineScope = "Runtime"
	local path = GetProjectBuildFilePath(ProjectName, engineScope)

	if FileExists(path) == false then
		engineScope = "Editor"
		path = GetProjectBuildFilePath(ProjectName, engineScope)
		if FileExists(path) == false then
			BuildFileLoadingError("'Source/[Runtime|Editor]/" .. projectName .. "/" .. projectName .. ".build.lua", "Build file not found!")
		end
	end

	return engineScope
end