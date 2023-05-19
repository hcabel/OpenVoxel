#include "Module.h"

AModule::AModule()
{
	m_IsLoaded = false;
	ModuleManager::RegisterNewModule(this);
}

void ModuleManager::RegisterNewModule(AModule* module)
{
	ModuleManager::Private::s_ModuleList.push_back(module);
}

void ModuleManager::LoadModules()
{
	for (auto& module : ModuleManager::Private::s_ModuleList)
	{
		module->StartupModule();
	}
}

void ModuleManager::UnloadModules()
{
	for (auto& module : ModuleManager::Private::s_ModuleList)
	{
		module->ShutdownModule();
	}
}
