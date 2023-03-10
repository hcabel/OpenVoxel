RuntimeModules = { "Core", "Engine" }
EditorModules = { "Editor" }

project "OpenVoxel"
	kind "ConsoleApp"
	language "C++"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

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
		-- Config files
		".editorconfig",
		".gitignore",
		".gitmodules",
		"premake5.lua",
		"premake_OpenVoxel.lua",
		"imgui.ini",

		-- Source files
		"Source/Runtime/**.h",
		"Source/Runtime/**.cpp",
	}


	includedirs
	{
		-- Third party
		"%{VULKAN_SDK}/Include",
		"Source/ThirdParty/imgui/include",
		"Source/ThirdParty/GLFW/include",
		"Source/ThirdParty/glm",
		"Source/ThirdParty/stb_image",

		-- Modules
		table.translate(RuntimeModules, function (moduleName) return ("Source/Runtime/" .. moduleName .. "/Public") end),
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
			"Source/Editor/**.h",
			"Source/Editor/**.cpp",
		}

		includedirs
		{
			-- Modules
			table.translate(EditorModules, function (moduleName) return ("Source/Editor/" .. moduleName .. "/Public") end),
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

	filter "configurations:*Release"
		defines
		{
			"OV_RELEASE"
		}

	-- System specific

	filter "system:windows"
		defines
		{
			"OV_PLATFORM_WINDOWS"
		}

	filter "system:linux"
		defines
		{
			"OV_PLATFORM_LINUX"
		}

	filter "system:macosx"
		defines
		{
			"OV_PLATFORM_MAC"
		}
	filter {}