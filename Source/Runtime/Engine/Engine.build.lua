
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

	Engine.ModulesDependencies = {
		"Renderer",
		"Core",
	}
	Engine.LibrariesDependencies = {
		"Vulkan",
		"GLFW",
		"glm",
	}

	Engine.Defines = {
		"OV_BUILD_ENGINE_DLL"
	}

	return Engine;
end

return EngineModule;