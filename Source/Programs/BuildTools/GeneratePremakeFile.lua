include "GlobalValues.lua"
include "ModuleDataResolver.lua"

function CreateModuleProject(moduleData)
	print("Creating module project: " .. moduleData.Name)

	project (moduleData.Name)
		UseModuleDefaultConfig()

		files(moduleData.Files)

		includedirs (moduleData.Public_IncludeDirs)
		includedirs (moduleData.Private_IncludeDirs)

		links (moduleData.ModulesDependencies)
		links (moduleData.LibrariesDependencies)

		defines (moduleData.Defines)
end

function GenerateModuleProject(moduleNameList)
	local modulesData = GetModulesData(moduleNameList)

	for moduleName, moduleData in pairs(modulesData) do
		CreateModuleProject(moduleData)
	end
end