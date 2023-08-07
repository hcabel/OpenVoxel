include "GlobalValues.lua"
include "BuildFile/Resolver.lua"
include "ProjectCreatorUtils.lua"

-- Generate a Premake's project for an application (console application .exe)
function CreateApplicationProject(buildData)

	project (buildData.Name)
		UseProjectDefaultConfig()

		DoXForEveryConfig(function (data, configuration)
			if configuration == "Common" then
				filter {} -- no filter (common data)
			else
				filter ("configurations:" .. configuration)
			end

				files(data.Files)

				includedirs (data.Resolved.Public_IncludeDirs)
				includedirs (data.Public_IncludeDirs)
				includedirs (data.Private_IncludeDirs)

				links (data.ModuleDependency)
				links (data.Resolved.ThirdPartyDependency)

				defines (data.Defines)

				postbuildcommands {
					table.translate(data.ModuleDependency,
						function (moduleName)
							return ('{COPY} "' .. MODULE_OUTPUT_PATH .. moduleName .. '.dll" "' .. PROJECT_OUTPUT_DIR .. '"')
						end
					)
				}
			filter {}
		end, buildData)
end

-- Generate a Premake's project for a module (static library .dll)
function CreateModuleProject(buildData)

	print("Generating project: " .. buildData.Name)
	project (buildData.Name)
		UseModuleDefaultConfig()

		DoXForEveryConfig(function (data, configuration)
			if configuration ~= "Common" then
				filter ("configurations:" .. configuration)
			end

				files(data.Files)

				includedirs (data.Resolved.Public_IncludeDirs)
				includedirs (data.Public_IncludeDirs)
				includedirs (data.Private_IncludeDirs)

				links (data.ModuleDependency)
				links (data.Resolved.ThirdPartyDependency)

				defines (data.Defines)

			filter {} -- Reset the filter
		end, buildData)
end

local GenerateProjectTable = {
	["Application"] = CreateApplicationProject,
	["Module"] = CreateModuleProject,
}

function GenerateModulesProject(moduleNameList)

	-- Loop through every module and generate the Premake's project for it
	for _, moduleName in ipairs(moduleNameList) do
		-- Get the module's build data
		local moduleData = GetBuildData(moduleName)

		-- Call the function that will generate the project for the module (depending on the module's kind)
		if moduleData.Kind and GenerateProjectTable[moduleData.Kind] then
			GenerateProjectTable[moduleData.Kind](moduleData)
		else
			error("Failed to generate project for module: " .. moduleName .. ". Module 'Kind' is not (or not well) defined. ('Application' or 'Module')")
		end
	end
end