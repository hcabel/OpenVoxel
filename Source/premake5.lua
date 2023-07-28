include "../vendor/Premake5/Utils.lua"

OV_RuntimeModules = { "Core", "Engine", "Renderer" }
OV_EditorModules = { "Editor", "UI" }

project "OpenVoxel"
	kind "ConsoleApp"

	UseProjectDefaultConfig()

	staticruntime "off"
	systemversion "latest"

	-- /* RUNTIME & EDITOR ****************************************************/

	files
	{
		"Runtime/Launch/**.h",
		"Runtime/Launch/**.cpp",
	}

	includedirs
	{
		"Runtime/Launch/Public",
		"Runtime/Launch/Private",

		-- Third party
		"%{VULKAN_SDK}/Include",
		"ThirdParty/imgui/include",
		"ThirdParty/GLFW/include",
		"ThirdParty/glm",

		-- Modules
		table.translate(OV_RuntimeModules, function (moduleName) return ("Runtime/" .. moduleName .. "/Public") end),
	}

	links
	{
		-- Third party
		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
		"ImGui",
		"GLFW",

		-- Runtime Modules
		table.translate(OV_RuntimeModules, function (moduleName) return (moduleName) end),
	}

	postbuildcommands {
		table.translate(OV_RuntimeModules, function (moduleName) return ('{COPY} "' .. ModuleTargetOutput .. '/' .. moduleName .. '.dll" "%{cfg.targetdir}"') end),
	}

	-- /* EDITOR *************************************************************/

	filter "configurations:Editor*"
		includedirs
		{
			-- Modules
			table.translate(OV_EditorModules, function (moduleName) return ("Editor/" .. moduleName .. "/Public") end),
		}

		links
		{
			table.translate(OV_EditorModules, function (moduleName) return (moduleName) end),
		}

		postbuildcommands {
			table.translate(OV_EditorModules, function (moduleName) return ('{COPY} "' .. ModuleTargetOutput .. '/' .. moduleName .. '.dll" "%{cfg.targetdir}"') end),
		}
	filter {}
