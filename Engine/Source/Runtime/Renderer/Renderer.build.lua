
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
		"Vulkan",
	}
	Renderer.ThirdPartyDependency = {
		"GLFW",
		"glm",
		"stb_image"
	}

	return Renderer;
end

return RendererModule;
