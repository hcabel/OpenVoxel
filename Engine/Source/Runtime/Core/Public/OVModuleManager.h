#pragma once

#include "Core_API.h"
#include "Logging/LoggingMacros.h"
#include "CoreGlobals.h"
#include "Module.h"

#include <unordered_map>

DECLARE_LOG_CATEGORY(ModuleManagerLog);

#define MODULEMANAGER_LOG(Verbosity, Format, ...) \
	OV_LOG(ModuleManagerLog, Verbosity, Format, __VA_ARGS__)

typedef Module* (*CreateModuleFunctionPtr)(void);

/**
 * This class is a base for all the platform module managers.
 * here you'll find some behavior that are not platform specific
 * and virtual function for the behavior that are platform specific.
 */
class CORE_API OVModuleManager
{
protected:
	OVModuleManager()
		: m_Modules()
	{}
	~OVModuleManager();

public:
	static OVModuleManager& Get()
	{
		static OVModuleManager instance;
		return instance;
	}

public:
	void Load(const char* moduleName);
	void Unload(const char* moduleName);

protected:
	std::unordered_map<const char*, Module*> m_Modules;
};
