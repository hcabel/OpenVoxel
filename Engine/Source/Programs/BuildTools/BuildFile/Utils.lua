
include "Cache.lua"

function DoXForEveryConfig(x, buildData)
	x(buildData["Common"], "Common")
	for _, configuration in ipairs(WKS.configurations) do
		x(buildData[configuration], configuration)
	end
end

-- Constants for the loading steps
MODULE_LOADED = 1
MODULE_RESOLVED = 2

function IsLoaded(moduleNameOrBuildData)
	if not moduleNameOrBuildData then
		return false
	end

	if type(moduleNameOrBuildData) == "string" then
		local buildData = GetBuildDataCached(moduleNameOrBuildData)
		return buildData and (buildData.LoadingSteps == MODULE_LOADED or buildData.LoadingSteps == MODULE_RESOLVED)
	else
		return (moduleNameOrBuildData.LoadingSteps == MODULE_LOADED or moduleNameOrBuildData.LoadingSteps == MODULE_RESOLVED)
	end
end

function IsResolved(moduleNameOrBuildData)
	if not moduleNameOrBuildData then
		return false
	end

	if type(moduleNameOrBuildData) == "string" then
		local buildData = GetBuildDataCached(moduleNameOrBuildData)
		return buildData and buildData.LoadingSteps == MODULE_RESOLVED
	else
		return moduleNameOrBuildData.LoadingSteps == MODULE_RESOLVED
	end
end

function CreateEmptyBuildData()
	return {
		Kind = nil,
		Files = {},
		Public_IncludeDirs = {},
		Private_IncludeDirs = {},
		ModuleDependency = {},
		ThirdPartyDependency = {},
		Defines = {},

		-- Extra data (Added by the resolver)
		Name = nil,
		RootDirectory = nil,
		EngineScope = nil,
		LoadingSteps = nil,
	}
end

-- Get all the build data for a give field and configuration
-- E.g if fieldName is "Public_IncludeDirs" and configuration is "Runtime - Debug"
--   it should return the "Public_IncludeDirs" from the "Runtime - Debug" configuration and the common one
function GetFieldFromBuildData(buildData, fieldName, configuration)
	local result = {}

	for _, value in ipairs(buildData["Common"][fieldName] or {}) do
		table.insert(result, value)
	end

	if configuration and configuration ~= "Common" and buildData[configuration] and buildData[configuration][fieldName] then
		for _, value in ipairs(buildData[configuration][fieldName]) do
			table.insert(result, value)
		end
	end

	return result
end
