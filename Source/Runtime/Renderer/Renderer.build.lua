
function RendererModule(config)
	Renderer = {}

	Renderer.Files = {
		"**.h",
		"**.cpp"
	}

	Renderer.Public_IncludeDirs = {
		"Public",
	}
	Renderer.Private_IncludeDirs = {
		"Private",

		-- REMOVE
		"../../ThirdParty/GLFW/include",
		"../../ThirdParty/glm",
		"%{VULKAN_SDK}/Include",
		"../Core/Public",
	}

	Renderer.ModulesDependencies = {
		"Core",
	}
	Renderer.LibrariesDependencies = {
		"%{VULKAN_SDK}/Lib/vulkan-1.lib", -- replace by "Vulkan",
		"GLFW",
		-- "glm",
	}

	Renderer.Defines = {
		"OV_BUILD_RENDERER_DLL"
	}

	return Renderer;
end

return RendererModule;