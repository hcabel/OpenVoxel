include "Premake/Environment.lua" -- Allow use to get access to the Premake workspace and request all the configuration
include "TableUtils.lua"

CONFIG_COMMON_KEY = "Common"

Config = {
	CONFIG_COMMON_KEY = {},
	-- ... 1 entry for each workspace configuration
}

function Config:new(InitializerFunc)
	local instance = {}

	setmetatable(instance, self)
	self.__index = self

	Config:ForEach(function (Config)
		if InitializerFunc ~= nil and type(InitializerFunc) == "function" then
			instance[Config] = InitializerFunc(Config)
		else
			instance[Config] = {}
		end
	end)

	return instance
end

function Config:GetAllConfigurations(FuncFilter)
	local result = {}

	local function AddToResult(value)
		if FuncFilter == nil or FuncFilter(value) then
			table.insert(result, value)
		end
	end

	AddToResult(CONFIG_COMMON_KEY)
	for _, configuration in ipairs(WKS.configurations) do
		AddToResult(configuration)
	end

	return result
end

-- Utils that will loop over all the configuration and execute the function with the current configuration
function Config:ForEach(Func)
	for _, configuration in ipairs(Config:GetAllConfigurations()) do
		Func(configuration)
	end
end

function Config:Get(Key, Configuration)
	if Configuration == nil then
		Configuration = CONFIG_COMMON_KEY
	end

	local result = self[CONFIG_COMMON_KEY][Key]

	if Config ~= CONFIG_COMMON_KEY then
		if type(result) == "table" then
			result = CombineTable(result, self[Configuration][Key]) -- combine common and config value if it's a table
		else
			result = self[Configuration][Key] or result -- override with the config value if it exist
		end
	end

	return result
end

function Config:Set(Key, Value, Configuration)
	if Configuration == nil then
		Configuration = CONFIG_COMMON_KEY
	end

	if self[Configuration] == nil then
		self[Configuration] = {}
	end

	if Configuration ~= CONFIG_COMMON_KEY then
		-- Check if it's not already in the Common configuration
		if type(self[CONFIG_COMMON_KEY][Key]) == "table" then
			for _, value in ipairs(self[CONFIG_COMMON_KEY][Key]) do
				if value == Value then
					return
				end
			end
		else
			if self[CONFIG_COMMON_KEY][Key] == Value then
				return
			end
		end
	end

	-- Check if it's not already in the configuration
	if type(self[Configuration][Key]) == "table" then
		for _, value in ipairs(self[Configuration][Key]) do
			if value == Value then
				return
			end
		end
	else
		if self[Configuration][Key] == Value then
			return
		end
	end

	if type(self[Configuration][Key]) == "table" then
		table.insert(self[Configuration][Key], Value)
	else
		self[Configuration][Key] = Value
	end
end

function Config:Add(Key, Value, Configuration)
	if Configuration == nil then
		Configuration = CONFIG_COMMON_KEY
	end

	if self[Configuration] == nil then
		self[Configuration] = {}
	end

	if self[Configuration][Key] == nil then
		self:Set(Key, Value, Configuration)
	else
		if type(self[Configuration][Key]) == "table" then
			if type(Value) == "table" then
				for _, value in ipairs(Value) do
					self:Set(Key, value, Configuration)
				end
			else
				self:Set(Key, Value, Configuration)
			end
		else
			if type(Value) == "table" then
				table.insert(Value, self[Configuration][Key])
				self:Set(Key, Value, Configuration)
			else
				self:Set(Key, { self[Configuration][Key], Value }, Configuration)
			end
		end
	end
end

ConfigEditorOnly = {}
setmetatable(ConfigEditorOnly, { __index = Config })

function ConfigEditorOnly:Add(Key, Value, Configuration)
	if Configuration == CONFIG_COMMON_KEY then -- Perform the add action on every editor configuration instead of the common one
		local allEditorConfigurations = Config:GetAllConfigurations(function (Config) return Config:find("Editor") end)
		for _, configuration in ipairs(allEditorConfigurations) do
			self:Add(Key, Value, configuration) -- recursive call
		end
	else -- Behave has usual (this is a copy of the Config:Add method, I couldn't figure out how to call it from this class)
		if self[Configuration] == nil then
			self[Configuration] = {}
		end

		if self[Configuration][Key] == nil then
			self:Set(Key, Value, Configuration)
		else
			if type(self[Configuration][Key]) == "table" then
				if type(Value) == "table" then
					for _, value in ipairs(Value) do
						self:Set(Key, value, Configuration)
					end
				else
					self:Set(Key, Value, Configuration)
				end
			else
				if type(Value) == "table" then
					self:Set(Key, Value, Configuration)
				else
					self:Set(Key, { self[Configuration][Key], Value }, Configuration)
				end
			end
		end
	end
end


