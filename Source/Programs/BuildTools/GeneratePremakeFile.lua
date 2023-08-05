include "GlobalValues.lua"
include "ModuleDataResolver.lua"
include "ProjectCreatorUtils.lua"

function CreateApplicationProject(applicationData)
	print("Creating application project: " .. applicationData.Name)

	project (applicationData.Name)
		UseProjectDefaultConfig()

		files(applicationData.Files)

		includedirs (applicationData.Linked.IncludeDirs)
		includedirs (applicationData.Private_IncludeDirs)

		links (applicationData.ModulesDependencies)
		links (applicationData.Linked.Links)

		defines (applicationData.Defines)

		postbuildcommands {
			table.translate(applicationData.ModulesDependencies,
				function (moduleName)
					return ('{COPY} "' .. MODULE_OUTPUT_PATH .. moduleName .. '.dll" "' .. PROJECT_OUTPUT_DIR .. '"')
				end
			)
		}
end

function CreateModuleProject(moduleData)
	print("Creating module project: " .. moduleData.Name)

	project (moduleData.Name)
		UseModuleDefaultConfig()

		-- If the module is an editor module, only add it to the editor configurations
		-- This way the module will not be built for the runtime configurations
		if moduleData.EngineScope == "Editor" then
			filter "configurations:Editor*"
		end

		files(moduleData.Files)

		includedirs (moduleData.Linked.IncludeDirs)
		includedirs (moduleData.Private_IncludeDirs)

		links (moduleData.ModulesDependencies)
		links (moduleData.Linked.Links)

		defines (moduleData.Defines)

		if moduleData.EngineScope == "Editor" then
			filter {}
		end
end

local GenerateProjectTable = {
	["Application"] = CreateApplicationProject,
	["Module"] = CreateModuleProject,
}

function GenerateModuleProject(moduleNameList)
	local modulesData = GetModulesData(moduleNameList)

	for moduleName, moduleData in pairs(modulesData) do
		if moduleData.Kind and GenerateProjectTable[moduleData.Kind] then
			GenerateProjectTable[moduleData.Kind](moduleData)
		else
			error("Failed to generate project for module: " .. moduleName .. ". Module 'Kind' is not defined. ('Application' or 'Module')")
		end
	end
end