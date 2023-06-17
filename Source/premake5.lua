RuntimeModules = { "Core", "Engine", "Renderer", "Launch" }
EditorModules = { "Editor" }

project "OpenVoxel"
	kind "ConsoleApp"
	language "C++"
	location (projectFileLocation)

	targetdir (buildOutput .. "/%{prj.name}")
	objdir (intermediateOutput .. "/%{prj.name}")

	-- TODO: Handle other OS (currently windows only)
	cppdialect "C++20"
	staticruntime "on"
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
		"Runtime/**.h",
		"Runtime/**.cpp",
	}


	includedirs
	{
		-- Third party
		"%{VULKAN_SDK}/Include",
		"ThirdParty/imgui/include",
		"ThirdParty/GLFW/include",
		"ThirdParty/glm",
		"ThirdParty/stb_image",

		-- Modules
		table.translate(RuntimeModules, function (moduleName) return ("Runtime/" .. moduleName .. "/Public") end),
	}

	links
	{
		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
		"ImGui",
		"GLFW",
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
			table.translate(EditorModules, function (moduleName) return ("Editor/" .. moduleName .. "/Public") end),
		}
	filter {}

	-- /* DEFINES ************************************************************/

	defines
	{
		"OV_BUILD_DLL",
	}

	-- Config specific

	filter "configurations:Editor*"
		defines
		{
			"WITH_EDITOR"
		}

	filter "configurations:*Debug"
		defines
		{
			"OV_DEBUG"
		}

	-- System specific

	filter "system:windows"
		defines
		{
			"PLATFORM_WINDOWS"
		}

	filter "system:linux"
		defines
		{
			"PLATFORM_LINUX"
		}

	filter "system:macosx"
		defines
		{
			"PLATFORM_MAC"
		}
	filter {}