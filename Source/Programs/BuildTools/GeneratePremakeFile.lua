include "GlobalValues.lua"
include "ModuleDataResolver.lua"
include "ProjectCreatorUtils.lua"

function CreateModuleProject(moduleData)
	print("Creating module project: " .. moduleData.Name)

	project (moduleData.Name)
		UseModuleDefaultConfig()

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

function GenerateModuleProject(moduleNameList)
	local modulesData = GetModulesData(moduleNameList)

	for moduleName, moduleData in pairs(modulesData) do
		CreateModuleProject(moduleData)
	end
end