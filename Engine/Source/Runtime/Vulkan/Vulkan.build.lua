
function RendererCoreModule(config)
	RendererCore = {}

	RendererCore.Public_IncludeDirs = {
		"Public",
		"Public/Context"
	}
	RendererCore.Private_IncludeDirs = {
		"Private",
	}

	RendererCore.ModuleDependency = {
		"Core",
	}
	RendererCore.ThirdPartyDependency = {
		"Vulkan",
	}

	return RendererCore;
end

return RendererCoreModule;
