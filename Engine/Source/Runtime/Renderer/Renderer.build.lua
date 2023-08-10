
function RendererModule(config)
	Renderer = {}

	Renderer.Public_IncludeDirs = {
		"Public",
	}
	Renderer.Private_IncludeDirs = {
		"Private",
	}

	Renderer.ModuleDependency = {
		"Core",
	}
	Renderer.ThirdPartyDependency = {
		"Vulkan",
		"GLFW",
		"glm",
	}

	Renderer.Defines = {
		"OV_BUILD_RENDERER_DLL"
	}

	return Renderer;
end

return RendererModule;