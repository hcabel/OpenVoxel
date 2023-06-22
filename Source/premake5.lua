OV_RuntimeModules = { "Core", "Engine", "Renderer" }
OV_EditorModules = { "Editor" }

project "OpenVoxel"
	kind "ConsoleApp"
	language "C++"
	location (projectFileLocation)

	targetdir (buildOutput .. "/%{prj.name}")
	objdir (intermediateOutput .. "/%{prj.name}")

	-- TODO: Handle other OS (currently windows only)
	cppdialect "C++20"
	staticruntime "off"
	systemversion "latest"

	filter "configurations:*Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:*Release"
		runtime "Release"
		optimize "on"
		symbols "off"
	filter {}

	-- /* RUNTIME ************************************************************/

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

	-- /* EDITOR *************************************************************/

	filter "configurations:Editor*"
		files
		{
			"Editor/**.h",
			"Editor/**.cpp",
		}

		includedirs
		{
			-- Modules
			table.translate(OV_EditorModules, function (moduleName) return ("Editor/" .. moduleName .. "/Public") end),
		}
	filter {}

	-- /* DEFINES ************************************************************/

	-- Config specific
	filter "configurations:Editor*"
		defines "WITH_EDITOR"
	filter "configurations:*Debug"
		defines "OV_DEBUG"

	-- System specific
	filter "system:windows"
		defines "PLATFORM_WINDOWS"
	filter "system:linux"
		defines "PLATFORM_LINUX"
	filter "system:macosx"
		defines "PLATFORM_MAC"
	filter {}
