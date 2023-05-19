#include "Module.h"

AModule::AModule(LoadingPhase::Type loadingPhase)
{
	ModuleManager::RegisterNewModule(this, loadingPhase);
}

void ModuleManager::RegisterNewModule(AModule* module, LoadingPhase::Type loadingPhase)
{
	ModuleManager::Private::s_Modules[loadingPhase].push_back(module);
}

void ModuleManager::LoadModules(LoadingPhase::Type loadingPhase)
{
	for (auto& module : ModuleManager::Private::s_Modules[loadingPhase])
	{
		module->StartupModule();
	}
}

void ModuleManager::UnloadModules()
{
	for (auto& module : ModuleManager::Private::s_Modules[LoadingPhase::Default])
	{
		module->ShutdownModule();
	}
}
