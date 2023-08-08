
include "../GlobalValues.lua"

local ThirdPartyDataCache = {}

function LoadThirdPartyFromDisk(thirdPartyName)
	print ("Loading '" .. thirdPartyName .. "' thirdParty from disk")

	local libRoot = ROOT_DIR_PATH .. "Source/ThirdParty/" .. thirdPartyName .. "/"
	local libPath = libRoot .. thirdPartyName .. ".link.lua"

	local thirdPartyData = {}
	if FileExists(libPath) then
		-- Load the thirdParty's link file
		local getThirdPartyDataFunc = dofile(libPath)
		if getThirdPartyDataFunc == nil then
			error("Failed to load thirdParty link file: " .. thirdPartyName .. " at path: " .. libPath .. ". File does not return the create thirdParty function.")
		end

		-- Get the thirdParty data
		thirdPartyData = getThirdPartyDataFunc()
		if not thirdPartyData then
			error("Failed to load thirdParty link file: " .. thirdPartyName .. " at path: " .. libPath .. ". Create thirdParty function did not return any valid thirdParty data.")
		end
	end

	-- Add extra handy data
	thirdPartyData.Name = thirdPartyName
	thirdPartyData.RootDirectory = thirdPartyData.RootDirectoryOverride or libRoot

	return thirdPartyData
end

function GetThirdPartyData(thirdPartyName)
	if ThirdPartyDataCache[thirdPartyName] == nil then
		ThirdPartyDataCache[thirdPartyName] = LoadThirdPartyFromDisk(thirdPartyName)
	end

	return ThirdPartyDataCache[thirdPartyName]
end

function LinkToThirdParty(thirdPartyName, buildData)
	local thirdPartyData = GetThirdPartyData(thirdPartyName)

	-- Add thirdParty public include directories
	buildData.Resolved.Public_IncludeDirs = buildData.Resolved.Public_IncludeDirs or {}
	for _, includeDir in ipairs(thirdPartyData.IncludeDirs or {}) do
		table.insert(buildData.Resolved.Public_IncludeDirs, thirdPartyData.RootDirectory .. includeDir)
	end
	table.insert(buildData.Resolved.Public_IncludeDirs, thirdPartyData.RootDirectory)

	buildData.Resolved.ThirdPartyDependency = buildData.Resolved.ThirdPartyDependency or {}
	table.insert(buildData.Resolved.ThirdPartyDependency, thirdPartyData.LinkName)
end