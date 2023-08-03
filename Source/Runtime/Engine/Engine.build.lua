
function EngineModule(config)
	Engine = {}

	Engine.Files = {
		"**.h",
		"**.cpp"
	}

	Engine.Public_IncludeDirs = {
		"Public",
	}
	Engine.Private_IncludeDirs = {
		"Private",

		-- REMOVE
		"%{VULKAN_SDK}/Include",
		"../../ThirdParty/GLFW/include",
		"../../ThirdParty/glm",
	}

	Engine.ModulesDependencies = {
		"Renderer",
		"Core",
	}
	Engine.LibrariesDependencies = {
		"%{VULKAN_SDK}/Lib/vulkan-1.lib", -- replace by "Vulkan",
		"GLFW",
		-- "glm",
	}

	Engine.Defines = {
		"OV_BUILD_ENGINE_DLL"
	}

	return Engine;
end

return EngineModule;