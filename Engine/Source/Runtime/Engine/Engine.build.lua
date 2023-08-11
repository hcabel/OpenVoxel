
function EngineModule(config)
	Engine = {}

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

	return Engine;
end

return EngineModule;