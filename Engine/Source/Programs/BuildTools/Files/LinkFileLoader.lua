include "../Constants.lua"
include "Utils.lua"
include "../Logger.lua"

local LinkFileCache = {}

local function LinkFileLoadingError(LinkFilePath, ErrorMessage)
	Logger:Error("Failed to load link file: '" .. (LinkFilePath or "") .. "'. " .. (ErrorMessage or ""))
end

local function LoadThirdPartyLinkFileFromDisk(LinkFilePath)
	local thirdPartyName = GetPathTarget(LinkFilePath)

	if FileExists(LinkFilePath) == false then
		-- If it doesn't exist we skip it, some default data will be used
		return function () return {} end
	end
	Logger:Start("Loading link file \"" .. thirdPartyName .. "\" from disk...")

	-- Load the module's link file, getLinkDataFunc is what has been returned by the file
	local getLinkDataFunc = dofile(LinkFilePath)
	if getLinkDataFunc == nil then
		LinkFileLoadingError(LinkFilePath, "File does not return the get link data function.")
	end

	Logger:End(" Done")
	return getLinkDataFunc or function () return {} end
end

function LoadThirdPartyLinkFile(ThirdPartyPath)
	if LinkFileCache[thirdPartyName] then
		return LinkFileCache[thirdPartyName]
	end

	local linkFile = LoadThirdPartyLinkFileFromDisk(ThirdPartyPath)
	LinkFileCache[ThirdPartyPath] = linkFile

	return linkFile
end

function GetThirdPartyLinkFilePath(ThirdPartyName)
	return GetThirdPartyRootPath(ThirdPartyName) .. ThirdPartyName .. ".link.lua"
end

function GetThirdPartyRootPath(ThirdPartyName)
	return ROOT_DIR_PATH .. "Source/ThirdParty/" .. ThirdPartyName .. "/"
end