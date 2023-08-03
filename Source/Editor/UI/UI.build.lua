
function UIModule(config)
	local UI = {}

	UI.Files = {
		"**.h",
		"**.cpp"
	}

	UI.Public_IncludeDirs = {
		"Public",
	}
	UI.Private_IncludeDirs = {
		"Private",

		-- REMOVE
		"../../ThirdParty/glm",
		"../../ThirdParty/GLFW/include",
		"%{VULKAN_SDK}/Include",
		"../../ThirdParty/imgui"
	}

	UI.ModulesDependencies = {
		"Core",
		"Renderer",
	}
	UI.LibrariesDependencies = {
		"GLFW",
		"%{VULKAN_SDK}/Lib/vulkan-1.lib", -- replace by "Vulkan",
		"ImGui",
	}

	UI.Defines = {
		"OV_BUILD_UI_DLL"
	}

	return UI
end

return UIModule