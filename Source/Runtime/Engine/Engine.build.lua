
function EngineModule(config)
	Engine = {}

	Engine.Kind = "Module"

	Engine.Files = {
		"**.h",
		"**.cpp"
	}

	Engine.Public_IncludeDirs = {
		"Public",
	}
	Engine.Private_IncludeDirs = {
		"Private",
	}

	Engine.ModuleDependency = {
		"Renderer",
		"Core",
	}
	Engine.ThirdPartyDependency = {}

	Engine.Defines = {
		"OV_BUILD_ENGINE_DLL"
	}

	return Engine;
end

return EngineModule;