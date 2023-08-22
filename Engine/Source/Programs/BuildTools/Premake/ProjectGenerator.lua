include "Constants.lua"
include "ProjectCreatorUtils.lua"
include "../Logger.lua"
include "../Files/Utils.lua"
include "../Configurations.lua"
include "../Project.lua"

-- Generate a Premake's project for an application (console application .exe)
function CreateApplicationProject(ProjectApp)

	Logger:Start("Generating project: \"" .. ProjectApp.Name .. "\"")
	-- ProjectApp:PrintBuildData()

	-- TODO: find another method to depend on all the modules
	--   because right now it's all taking all the application
	local allRuntimeModules = GetFolders(ROOT_DIR_PATH .. "Source/Runtime/")
	local allEditorModules = GetFolders(ROOT_DIR_PATH .. "Source/Editor/")

	project (ProjectApp.Name)
		UseProjectDefaultConfig()

		Config:ForEach(function (Configuration)
			if Configuration ~= "Common" then -- We discard the common configuration because those data will be added to every configuration
				filter ("configurations:" .. Configuration)

					-- Application will also build all the modules, just in case they are loaded by the module manager (which don't require linking)
					dependson (allRuntimeModules)
					if Configuration:find("Editor") then
						dependson (allEditorModules)
					end

					-- We include all the file under the module directory
					files({
						ProjectApp.RootDirectory .. "**.h",
						ProjectApp.RootDirectory .. "**.cpp",
						ProjectApp.RootDirectory .. "**.lua",
					})

					includedirs (ProjectApp.PerConfig_BuildData:Get("IncludeDirs", Configuration))
					links (ProjectApp.PerConfig_BuildData:Get("Links", Configuration))
					defines (ProjectApp.PerConfig_BuildData:Get("Defines", Configuration))
				filter {}
			end
		end)

	Logger:End(" Done")
end

-- Generate a Premake's project for a module (static library .dll)
function CreateModuleProject(ProjectModule)
	Logger:Start("Generating project: \"" .. ProjectModule.Name .. "\"")

	-- Get all config except the common one
	local configurations = Config:GetAllConfigurations( function (value) return value ~= CONFIG_COMMON_KEY end)

	project (ProjectModule.Name)
		UseModuleDefaultConfig()

		for _, configuration in ipairs(configurations) do
			filter ("configurations:" .. configuration)
				-- We include all the file under the module directory
				files(ProjectModule.PerConfig_BuildData:Get("Files", configuration))

				includedirs (ProjectModule.PerConfig_BuildData:Get("IncludeDirs", configuration))
				links (ProjectModule.PerConfig_BuildData:Get("Links", configuration))
				defines (ProjectModule.PerConfig_BuildData:Get("Defines", configuration))
			filter {}
		end

	Logger:End(" Done")
end

local GenerateProjectTable = {
	["Application"] = CreateApplicationProject,
	["Module"] = CreateModuleProject,
}

function GenerateProject(ProjectNameList)

	-- Loop through every module and generate the Premake's project for it
	for _, projectName in ipairs(ProjectNameList) do
		-- Get the module's build data
		local project = Project:Get(projectName)

		-- Call the function that will generate the project for the module (depending on the module's kind)
		if project.Kind and GenerateProjectTable[project.Kind] then
			GenerateProjectTable[project.Kind](project)
		else
			Logger:Error("Failed to generate project for module: " .. projectName .. ". Module 'Kind' is not (or not well) defined. ('Application' or 'Module')")
		end
	end
end