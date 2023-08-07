
local BuildDataCache = {}

function GetBuildDataCached(buildDataName)
	return BuildDataCache[buildDataName]
end

function CacheBuildData(buildDataName, buildData)
	BuildDataCache[buildDataName] = buildData
end

