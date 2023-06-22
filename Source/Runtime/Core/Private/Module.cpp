#include "Module.h"

std::unordered_map<LoadingPhase::Type, std::vector<AModule*>>	ModuleManager::s_Modules;

AModule::AModule(LoadingPhase::Type loadingPhase)
{
	ModuleManager::RegisterNewModule(this, loadingPhase);
}

void ModuleManager::RegisterNewModule(AModule* module, LoadingPhase::Type loadingPhase)
{
	s_Modules[loadingPhase].push_back(module);
}

void ModuleManager::LoadModules(LoadingPhase::Type loadingPhase)
{
	for (auto& module : s_Modules[loadingPhase])
	{
		module->StartupModule();
	}
}

void ModuleManager::UnloadModules()
{
	for (auto& module : s_Modules[LoadingPhase::Default])
	{
		module->ShutdownModule();
	}
}
