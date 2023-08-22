include "Files/LinkFileLoader.lua"

LinkFile = {
	["Name"] = "", -- the name of the library (e.g. "Vulkan")
	["LinkName"] = "", -- That that will be use to link the project (e.g. "C:/Vulkan/vulkan.lib")
	["IncludeDirs"] = {},
	["RootDirectory"] = "",
}

function LinkFile:Load()
	local path = GetThirdPartyRootPath(self.Name)

	local getLinkDataFunc = LoadThirdPartyLinkFile(GetThirdPartyLinkFilePath(self.Name))
	local linkData = getLinkDataFunc()

	-- Set the data
	self.LinkName = linkData.LinkName or ""
	self.RootDirectory = linkData.RootDirectory or path
	self.IncludeDirs = linkData.IncludeDirs or {}
end

function LinkFile:new(Name)
	local instance = {}
	setmetatable(instance, self)
	self.__index = self

	instance.Name = Name

	instance:Load()

	return instance
end

local ThirdPartyCache = {}

function LinkFile:Get(Name)
	if ThirdPartyCache[Name] == nil then
		ThirdPartyCache[Name] = LinkFile:new(Name)
	end

	return ThirdPartyCache[Name]
end