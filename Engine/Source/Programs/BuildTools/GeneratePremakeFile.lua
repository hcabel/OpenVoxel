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

				-- We include all the file under the module directory
				files({
					buildData.RootDirectory .. "**.h",
					buildData.RootDirectory .. "**.cpp",
					buildData.RootDirectory .. "**.lua",
				})

				includedirs (data.Resolved.Public_IncludeDirs)
				includedirs (data.Public_IncludeDirs)
				includedirs (data.Private_IncludeDirs)

				links (data.ModuleDependency)
				links (data.Resolved.ThirdPartyDependency)

				defines (data.Defines)

			filter {}
		end, buildData)
end

-- Generate a Premake's project for a module (static library .dll)
function CreateModuleProject(buildData)

	print("Generating project: " .. buildData.Name)
	project (buildData.Name)
		UseModuleDefaultConfig()

		DoXForEveryConfig(function (data, configuration)

			-- Add building data for Editor configuration only if the module is an Editor module
			if buildData.EngineScope == "Editor" and configuration:find("Editor") == nil then
				goto continue
			end

			if configuration ~= "Common" then
				filter ("configurations:" .. configuration)
			end

				files({
					buildData.RootDirectory .. "**.h",
					buildData.RootDirectory .. "**.cpp",
					buildData.RootDirectory .. "**.lua",
				})

				includedirs (data.Resolved.Public_IncludeDirs)
				includedirs (data.Public_IncludeDirs)
				includedirs (data.Private_IncludeDirs)

				links (data.ModuleDependency)
				links (data.Resolved.ThirdPartyDependency)

				defines (data.Defines)
				-- Add API define (OV_BUILD_[UpperCaseModuleName]_DLL)
				defines ("OV_BUILD_" .. string.upper(buildData.Name) .. "_DLL")

			filter {} -- Reset the filter

			::continue::
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