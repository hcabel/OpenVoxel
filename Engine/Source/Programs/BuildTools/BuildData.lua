include "FileBuildData.lua"
include "TableUtils.lua"

-- This is what the FileBuildData are turned into after the resolving process
BuildData = {
	IncludeDirs = {},
	Links = {},
	Defines = {},
}

function BuildData:new()
	local instance = {}
	setmetatable(instance, self)
	self.__index = self

	-- Set default value
	for key, value in pairs(BuildData) do
		if type(value) ~= "function" then
			if type(value) == "table" then
				instance[key] = {}
				for _, v in ipairs(value) do
					table.insert(instance[key], v)
				end
			else
				instance[key] = value
			end
		end
	end

	return instance
end

function BuildData:Combine(BuildDataA, BuildDataB)
	local buildDataResult = BuildData:new()

	-- Loop over each field and combine them
	for key, _ in pairs(buildDataResult) do
		if type(buildDataResult[key]) == "table" then
			buildDataResult[key] = CombineTable(BuildDataA[key], BuildDataB[key])
		else
			buildDataResult[key] = BuildDataA[key] or BuildDataB[key]
		end
	end
end