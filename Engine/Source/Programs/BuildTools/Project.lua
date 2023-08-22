include "Logger.lua"
include "FileBuildData.lua"
include "Configurations.lua"
include "BuildData.lua"
include "Files/BuildFileLoader.lua"
include "LinkFile.lua"

Project = {
	Name = "Unknown",
	Kind = "Module", -- (Module | Application)
	EngineScope = "Unknown", -- (Runtime | Editor)
	RootDirectory = "Unknown",

	-- The data use to generate the Premake's project one a per config basis (Will be modified by the linking process)
	PerConfig_BuildData = {
		-- [Config] = [BuildData]
	},
	-- The data fetch from the build file on a per config basis (Will not be modified)
	PerConfig_FileBuildData = {
		-- [Config] = [FileBuildData]
	},
}

function Project:Load()
	self.EngineScope = GetEngineScope(self.Name)
	self.RootDirectory = GetProjectRootPath(self.Name, self.EngineScope)

	local GetBuildDataFunc = LoadProjectBuildFile(GetProjectBuildFilePath(self.Name, self.EngineScope))

	self.PerConfig_FileBuildData = Config:new(function () return FileBuildData:new() end)

	Config:ForEach(function (Config)
		local configParameter = {
			configuration = (Config == CONFIG_COMMON_KEY and "" or Config),
			architecture = WKS.architecture, -- TODO: In the future will probably have to loop over all the architecture too
			platform = WKS.platform,
		}

		local newFileBuildData = GetBuildDataFunc(configParameter);

		-- Crappy way to catch Kind
		if Config == "Common" then
			self.Kind = newFileBuildData.Kind or self.Kind;
		end
		newFileBuildData = FileBuildData:new(newFileBuildData) -- at this point we get rid of value that are not in FileBuildData (Kind included)

		-- Add the data to the per config table, but remove the data that are common to all configuration
		self.PerConfig_FileBuildData[Config] = newFileBuildData:ExcludeCommonData(self.PerConfig_FileBuildData["Common"] or {})
	end)

	-- self:PrintFileBuildData();
end

function Project:LinkToThirdParty(ThirdPartyName, Config)
	local thirdParty = LinkFile:Get(ThirdPartyName)
	local configBuildData = self.PerConfig_BuildData[Config]

	-- Add root directory to the include dir
	self.PerConfig_BuildData:Add("IncludeDirs", thirdParty.RootDirectory, Config)

	-- Add the include dir to the build data
	for _, includeDir in ipairs(thirdParty.IncludeDirs) do
		self.PerConfig_BuildData:Add("IncludeDirs", thirdParty.RootDirectory .. includeDir, Config)
	end

	-- Add the link to the build data
	if thirdParty.LinkName ~= "" then -- Handy for header only library
		self.PerConfig_BuildData:Add("Links", thirdParty.LinkName, Config)
	end
end

function Project:LinkToModule(ModuleName, Config)

	Logger:Push()
	local projectDependency = Project:Get(ModuleName)
	Logger:Pop()

	-- Add the link to the build data
	self.PerConfig_BuildData:Add("Links", projectDependency.Name, Config)

	-- Add the include dir to the build data
	for _, includeDir in ipairs(projectDependency.PerConfig_FileBuildData:Get("Public_IncludeDirs", Config)) do
		self.PerConfig_BuildData:Add("IncludeDirs", projectDependency.RootDirectory .. includeDir, Config)
	end

	for _, moduleDependencyName in ipairs(projectDependency.PerConfig_FileBuildData:Get("ModuleDependency", Config)) do
		if moduleDependencyName == ModuleName then
			Logger:Error("Module \"" .. ModuleName .. "\" is trying to link to itself")
		end
		self:LinkToModule(moduleDependencyName, Config)
	end

	for _, thirdPartyDependencyName in ipairs(projectDependency.PerConfig_FileBuildData:Get("ThirdPartyDependency", Config)) do
		self:LinkToThirdParty(thirdPartyDependencyName, Config)
	end
end

function Project:Link()
	Logger:Start("Linking project \"" .. self.Name .. "\"...")
	Logger:Push()

	if self:IsEditorProject() then
		-- For editor project we're gonna redirected Add action to the common config to every editor config
		self.PerConfig_BuildData = ConfigEditorOnly:new(function () return BuildData:new() end)
	else
		self.PerConfig_BuildData = Config:new(function () return BuildData:new() end)
	end

	-- Add files to the build data
	self.PerConfig_BuildData:Add("Files", {
		self.RootDirectory .. "**.h",
		self.RootDirectory .. "**.cpp",
		self.RootDirectory .. "**.lua",
	}, CONFIG_COMMON_KEY)

	-- Add the build dll define if it's a module
	if self.Kind == "Module" then
		self.PerConfig_BuildData:Add("Defines", "OV_BUILD_" .. string.upper(self.Name) .. "_DLL", CONFIG_COMMON_KEY)
	end

	-- Link the FileBuildData
	Config:ForEach(function (Config)
		-- Go through all the FileBuildData field and use an table of function to link them
		local Linker = {
			Public_IncludeDirs = function (IncludeDirs)
				for _, includeDir in ipairs(IncludeDirs) do
					self.PerConfig_BuildData:Add("IncludeDirs", self.RootDirectory .. includeDir, Config)
				end
			end,
			Private_IncludeDirs = function (IncludeDirs)
				for _, includeDir in ipairs(IncludeDirs) do
					self.PerConfig_BuildData:Add("IncludeDirs", self.RootDirectory .. includeDir, Config)
				end
			end,
			ModuleDependency = function (ModuleDependency)
				for _, moduleDependencyName in ipairs(ModuleDependency) do
					self:LinkToModule(moduleDependencyName, Config)
				end
			end,
			ThirdPartyDependency = function (ThirdPartyDependency)
				for _, thirdPartyDependencyName in ipairs(ThirdPartyDependency) do
					self:LinkToThirdParty(thirdPartyDependencyName, Config)
				end
			end,
			Defines = function (Defines)
				self.PerConfig_BuildData:Add("Defines", Defines, Config)
			end,
		}

		-- Link the FileBuildData
		for key, value in pairs(self.PerConfig_FileBuildData[Config]) do
			Linker[key](value)
		end
	end)

	Logger:Pop()
	Logger:End(" Done")
end

function Project:new(ProjectName)
	local instance = {}
	setmetatable(instance, self)
	self.__index = self

	Logger:Start("Loading project \"" .. ProjectName .. "\"...")
	Logger:Push()

	-- Set default value
	for key, value in pairs(Project) do
		if type(value) ~= "function" then
			instance[key] = value
		end
	end

	instance.Name = ProjectName

	-- Load project build files
	instance:Load()

	-- Convert FileBuildData to BuildData
	instance:Link()

	Logger:Pop()
	Logger:End(" Done")
	return instance
end

function Project:IsRuntimeProject()
	return self.EngineScope == "Runtime"
end

function Project:IsEditorProject()
	return self.EngineScope == "Editor"
end

function Project:PrintFileBuildData()
	Logger:Push("---- " .. self.Kind .. " \"" .. self.Name .. "\" (" .. self.EngineScope .. ") ----");
	Logger:Log("Root: \"" .. self.RootDirectory .. "\"")

	Logger:Push("[Private]: ")

	-- Print include dirs
	Logger:Push("IncludeDirs: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_FileBuildData[Config].Private_IncludeDirs > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, includeDirs in ipairs(self.PerConfig_FileBuildData[Config].Private_IncludeDirs) do
				Logger:Log(includeDirs)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end
		end
	end);
	Logger:Pop()

	-- Print module dependencies
	Logger:Push("ModuleDependency: ")
	Config:ForEach(function (Configuration)
		if #self.PerConfig_FileBuildData[Configuration].ModuleDependency > 0 then

			if Configuration ~= "Common" then
				Logger:Push("\"" .. Configuration .. "\":")
			end

			for _, dependency in ipairs(self.PerConfig_FileBuildData[Configuration].ModuleDependency) do
				Logger:Log("" .. dependency)
			end

			if Configuration ~= "Common" then
				Logger:Pop()
			end
		end
	end)
	Logger:Pop()


	-- Print third party dependencies
	Logger:Push("ThirdPartyDependency: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_FileBuildData[Config].ThirdPartyDependency > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, dependency in ipairs(self.PerConfig_FileBuildData[Config].ThirdPartyDependency) do
				Logger:Log("" .. dependency)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end
		end
	end)
	Logger:Pop()

	-- Print defines
	Logger:Push("Defines: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_FileBuildData[Config].Defines > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, define in ipairs(self.PerConfig_FileBuildData[Config].Defines) do
				Logger:Log("" .. define)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end

		end
	end)
	Logger:Pop(2)

	Logger:Push("[Public]: ")
	-- Print include dirs
	Logger:Push("IncludeDirs: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_FileBuildData[Config].Public_IncludeDirs > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, includeDirs in ipairs(self.PerConfig_FileBuildData[Config].Public_IncludeDirs) do
				Logger:Log(includeDirs)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end
		end
	end)
	Logger:Pop(3)
end

function Project:PrintBuildData()
	Logger:Push("---- " .. self.Kind .. " \"" .. self.Name .. "\" (" .. self.EngineScope .. ") ----");
	Logger:Log("Root: \"" .. self.RootDirectory .. "\"")

	-- Print include dirs
	Logger:Push("IncludeDirs: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_BuildData[Config].IncludeDirs > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, includeDirs in ipairs(self.PerConfig_BuildData[Config].IncludeDirs) do
				Logger:Log(includeDirs)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end
		end
	end)
	Logger:Pop()

	-- Print links dependencies
	Logger:Push("Links: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_BuildData[Config].Links > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, link in ipairs(self.PerConfig_BuildData[Config].Links) do
				Logger:Log("" .. link)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end
		end
	end)
	Logger:Pop()

	-- Print defines
	Logger:Push("Defines: ")
	Config:ForEach(function (Config)
		if #self.PerConfig_BuildData[Config].Defines > 0 then
			if Config ~= "Common" then
				Logger:Push("\"" .. Config .. "\":")
			end

			for _, define in ipairs(self.PerConfig_BuildData[Config].Defines) do
				Logger:Log("" .. define)
			end

			if Config ~= "Common" then
				Logger:Pop()
			end

		end
	end)
	Logger:Pop(2)
end


function Project:Print()
	Logger:w(dump(self, { "__index" }))
end

local ProjectCache = {}

function Project:Get(ProjectName)

	-- TODO: Use two loading step loading and link, so when requesting data for linking to a module we to link it we just load the file

	if ProjectCache[ProjectName] == nil then
		ProjectCache[ProjectName] = Project:new(ProjectName)
	end

	return ProjectCache[ProjectName]
end

