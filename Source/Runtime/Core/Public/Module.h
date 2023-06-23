#pragma once

#include "Core_API.h"
#include "MacrosHelper.h"

#include <unordered_map>
#include <vector>

namespace LoadingPhase
{
	enum Type : uint8_t
	{
		/** Will be loaded just before the default modules */
		PreDefault,
		/** Will be loaded at the start of the program before the engine but after the core modules */
		Default = 0,
		/** Will be loaded right after the default modules */
		PostDefault,

		/** Will be loaded before the engine initialisation */
		PreEngine,
		/** Will be loaded after the engine initialisation */
		PostEngine,
	};
}

/**
 * AModule is the base class for every module.
 */
class CORE_API AModule
{

public:
	AModule(LoadingPhase::Type loadingPhase = LoadingPhase::Default);
	~AModule() = default;

	AModule(const AModule&) = delete;
	AModule& operator=(const AModule&) = delete;

public:
	/** Called when the module is loaded */
	virtual void StartupModule() = 0;
	/** Called when unloading the module */
	virtual void ShutdownModule() = 0;
};

/**
 * The module manager is here to handle all the module.
 */
class CORE_API ModuleManager
{
public:
	/** Register a new module to the module list */
	static void RegisterNewModule(AModule* module, LoadingPhase::Type loadingPhase);
	/** Load all the registered modules of the specified phase */
	static void LoadModules(LoadingPhase::Type loadingPhase);
	/** Unload all the registered modules */
	static void UnloadModules();

private:
	static std::unordered_map<LoadingPhase::Type, std::vector<AModule*>> s_Modules;
};

#define REGISTER_MODULE(ModuleClass) inline AModule* JOIN(g_, ModuleClass) = new ModuleClass();
