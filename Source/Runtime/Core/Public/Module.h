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
		PreDefault = 1,
		/** Will be loaded at the start of the program before the engine but after the core modules */
		Default = 0,
		/** Will be loaded right after the default modules */
		PostDefault = 2,

		/** Will be loaded before the engine initialization */
		PreEngine = 3,
		/** Will be loaded after the engine initialization */
		PostEngine = 4,
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

// Macro to declare a module has an global extern variable
// Use this macro in the header file of the module
#define DECLARE_MODULE(ModuleClass) \
	extern AModule* g_##ModuleClass;

#include <iostream>

// Macro to define an extern variable for a module
// Use this macro in the cpp file of the module
#define DEFINE_MODULE(ModuleClass) \
	AModule* g_##ModuleClass = new ModuleClass(); \
	STATIC_CHECK_BASE_OF(AModule, ModuleClass);
