include "GlobalValues.lua"

function dump(object)
	if type(object) == 'table' then
		local s = '{ '
		for k,v in pairs(object) do
			if type(k) ~= 'number' then k = '"'..k..'"' end
			s = s .. '['..k..'] = ' .. dump(v) .. ','
		end
		return s .. '} '
	else
		return tostring(object)
	end
end

function DEBUG_PrintModuleData(moduleData)
	print("Module: " .. moduleData.Name)
	print("\tFiles: ")
	for _, filePath in ipairs(moduleData.Files) do
		print("\t\t" .. filePath)
	end
	print("\tPublic_IncludeDirs: ")
	for _, includeDirPath in ipairs(moduleData.Public_IncludeDirs) do
		print("\t\t" .. includeDirPath)
	end
	print("\tPrivate_IncludeDirs: ")
	for _, includeDirPath in ipairs(moduleData.Private_IncludeDirs) do
		print("\t\t" .. includeDirPath)
	end
	print("\tModulesDependencies: " .. table.concat(moduleData.ModulesDependencies, ", "))
	print("\tLibrariesDependencies: " .. table.concat(moduleData.LibrariesDependencies, ", "))
end

-- Replace all the path by the absolute path
function ResolveModulePaths(moduleData)
	-- Files
	for index, filePath in ipairs(moduleData.Files) do
		moduleData.Files[index] = moduleData.RootDirectory .. filePath
	end

	-- include directories
	for index, includeDirPath in ipairs(moduleData.Public_IncludeDirs) do
		moduleData.Public_IncludeDirs[index] = moduleData.RootDirectory .. includeDirPath
	end
	for index, includeDirPath in ipairs(moduleData.Private_IncludeDirs) do
		-- If start with "%{VULKAN_SDK}/Include" do nothing
		if string.find(includeDirPath, "%%{VULKAN_SDK}/Include") == nil then
			moduleData.Private_IncludeDirs[index] = moduleData.RootDirectory .. includeDirPath
		end
	end

	return moduleData
end

local config = {}; -- TODO

-- Store the modules that have already been created
local ModulesDataCache = {}

local MODULE_CREATED = 1
local MODULE_LINKED = 2
local MODULE_LOADED = 3

function LoadBuildFile(moduleName)
	if ModulesDataCache[moduleName] then
		return ModulesDataCache[moduleName]
	end

	local moduleFilePath = "Source/Runtime/" .. moduleName .. "/" .. moduleName .. ".build.lua"

	-- Load the module's build file
	local createModuleFunc = dofile(moduleFilePath)
	if createModuleFunc == nil then
		error("Failed to load module build file: " .. moduleName .. " at path: " .. moduleFilePath .. ". File does not export the createModule function.")
	end

	-- Create the module
	local moduleData = createModuleFunc(config)

	if moduleData == nil then
		error("Failed to load module build file: " .. moduleName .. " at path: " .. moduleFilePath .. ". Function did not return any module data.")
	end

	-- Add extra handy data
	moduleData.Name = moduleName
	moduleData.RootDirectory = ROOT_DIR_PATH .. "Source/Runtime/" .. moduleName .. "/"
	moduleData.LoadingSteps = MODULE_CREATED

	-- resolve all the paths to avoid any confusion
	moduleData = ResolveModulePaths(moduleData)

	-- Cache the module data (to avoid loading it twice)
	ModulesDataCache[moduleName] = moduleData
	return ModulesDataCache[moduleName]
end

function LinkModule(moduleName)
	print ("Link modules: " .. moduleName)

	local currentModule = ModulesDataCache[moduleName]

	if currentModule == nil then
		error("Failed to link module: " .. moduleName .. ". Module has not been loaded yet.")
	end
	if currentModule.LoadingSteps == MODULE_LINKED then
		return currentModule
	end

	currentModule.LoadingSteps = MODULE_LINKED
	return ModulesDataCache[moduleName]
end

function LoadModule(moduleName)
	print ("Load modules: " .. moduleName)

	-- Load the module's build file
	LoadBuildFile(moduleName)

	-- Linking the module with all it's dependencies
	LinkModule(moduleName);

	ModulesDataCache[moduleName].LoadingSteps = MODULE_LOADED
	return ModulesDataCache[moduleName]
end

function GetModuleData(moduleName)
	if ModulesDataCache[moduleName] and ModulesDataCache[moduleName].LoadingSteps == MODULE_LOADED then
		return ModulesDataCache[moduleName]
	end

	LoadModule(moduleName)

	return ModulesDataCache[moduleName]
end

function GetModulesData(moduleNameList)
	local modulesData = {}
	for _, moduleName in ipairs(moduleNameList) do
		table.insert(modulesData, GetModuleData(moduleName))
	end
	return modulesData
end