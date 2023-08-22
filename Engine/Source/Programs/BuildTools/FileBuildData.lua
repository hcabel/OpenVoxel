include "TableUtils.lua"

-- This is the data that is read from the build file
FileBuildData = {
	Public_IncludeDirs = {},
	Private_IncludeDirs = {},
	ModuleDependency = {},
	ThirdPartyDependency = {},
	Defines = {},
}

function FileBuildData:new(InFileBuildData)
	local instance = {}
	setmetatable(instance, self)
	self.__index = self

	-- if we have data, we populate the instance with only the data that can be accepted
	if InFileBuildData then
		for key, value in pairs(FileBuildData) do
			if type(instance[key]) ~= "function" and key ~= "__index" and key ~= "new" then
				instance[key] = InFileBuildData[key]
			end
		end
	end

	return instance
end

-- Compare source against exclude and return source without the exclude value
-- if both are table we remove the common data
-- if only source is a table, we remove the exclude value from the source table
-- if only exclude is a table, we make sure that source does not equal any of the exclude value
-- if none of them are tables, we just compare them and return nil if they're equal or source if they're not
local function ValueExcluder(source, exclude)
	if source == nil then
		return nil
	elseif exclude == nil then
		return source
	end

	if type(source) == "table" and type(exclude) == "table" then
		-- if they're both tables, we need to remove the common data
		local result = {}

		for key, value in pairs(source) do
			if type(key) == "number" then -- this allow to keep the array order otherwise they'll behave like they're empty
				local newValue = ValueExcluder(value, exclude[key])
				if newValue ~= nil then
					table.insert(result, newValue)
				end
			else
				result[key] = ValueExcluder(value, exclude[key])
			end
		end

		return result
	elseif type(source) ~= "table" and type(exclude) == "table" then
		-- If only exclude is a table, we make sure that source does not equal any of the exclude value
		local hasBeenFound = false

		for key, value in pairs(exclude) do
			if source == value then
				hasBeenFound = true
				break
			end
		end

		if hasBeenFound then
			return nil
		else
			return source
		end
	elseif type(source) == "table" and type(exclude) ~= "table" then
		-- If only source is a table, we remove the exclude value from the source table
		local result = {}

		for key, value in pairs(source) do
			if value ~= exclude then
				result[key] = value
			end
		end

		return result
	else
		-- If none of them are tables, we just compare them
		if source == exclude then
			return nil
		else
			return source
		end
	end
end

function FileBuildData:ExcludeCommonData(exclude)

	for key, value in pairs(exclude) do
		self[key] = ValueExcluder(self[key], value)
	end

	return self
end