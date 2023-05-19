#pragma once

#include "CoreModule.h"
#include "MacrosHelper.h"
#include <vector>

/**
 * AModule is the base class for every module.
 */
class CORE_API AModule
{

public:
	AModule();
	~AModule() = default;

	AModule(const AModule&) = delete;
	AModule& operator=(const AModule&) = delete;

public:
	/** Called when the module is loaded */
	virtual void StartupModule() = 0;
	/** Called when unloading the module */
	virtual void ShutdownModule() = 0;

private:
	bool m_IsLoaded;
};

/** 
 * The module manager is here to handle all the module.
 * TODO: Add module loading phases (default, early, ...)
 */
namespace ModuleManager
{
	namespace Private
	{
		inline std::vector<AModule*> s_ModuleList;
	}

	/** Register a new module to the module list */
	void RegisterNewModule(AModule *module);
	/** Load all the registered modules */
	void LoadModules();
	/** Unload all the registered modules */
	void UnloadModules();
}

#define REGISTER_MODULE(ModuleClass) inline AModule* JOIN(g_, ModuleClass) = new ModuleClass();

