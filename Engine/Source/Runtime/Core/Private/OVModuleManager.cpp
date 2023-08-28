#include "OVModuleManager.h"
#include "HAL/File.h"
#include "HAL/DLLFile.h"
#include "Path.h"

DEFINE_LOG_CATEGORY(ModuleManagerLog);

OVModuleManager::~OVModuleManager()
{
	// shutdown all remaining modules
	for (auto& module : m_Modules)
	{
		MODULEMANAGER_LOG(Warning, "Automatically shutting down module '{:s}'", module.first);
		module.second->ShutdownModule();
	}
}

void OVModuleManager::Load(const char *moduleName)
{
	auto moduleIt = m_Modules.find(moduleName);
	if (moduleIt != m_Modules.end())
	{
		MODULEMANAGER_LOG(Error, "Unable to load module '{:s}': module already loaded", moduleName);
		return;
	}

	Path modulePath = Path::GetModuleDirectoryPath().AppendSegment(std::string(moduleName) + ".dll");

	if (File::Exists(modulePath) == false)
	{
		MODULEMANAGER_LOG(Error, "Unable to load module '{:s}': module not found ('{:s}')", moduleName, std::string(modulePath));
		return;
	}

	DLLFile* moduleDLL = DLLFile::Load(modulePath);
	if (!moduleDLL->IsValid())
	{
		MODULEMANAGER_LOG(Error, "Unable to load module '{:s}': couldn't be loaded by the OS", moduleName);
		return;
	}

	CreateModuleFunctionPtr createModuleFunctionPtr = (CreateModuleFunctionPtr)moduleDLL->GetFunctionPtr("CreateModule");
	if (createModuleFunctionPtr == nullptr)
	{
		MODULEMANAGER_LOG(Error, "Unable to load module '{:s}': The module is not implemented (use the IMPLEMENT_MODULE macro)", moduleName);
		return;
	}

	Module* module = createModuleFunctionPtr();
	m_Modules.insert(std::make_pair(moduleName, module));
	module->StartupModule();


	MODULEMANAGER_LOG(Verbose, "Module '{:s}' has been loaded", moduleName);
}

void OVModuleManager::Unload(const char* moduleName)
{
	auto moduleIt = m_Modules.find(moduleName);
	if (moduleIt == m_Modules.end())
	{
		MODULEMANAGER_LOG(Error, "Unable to unload module '{:s}': module not found", moduleName);
		return;
	}

	Module* module = moduleIt->second;
	module->ShutdownModule();

	m_Modules.erase(moduleIt);

	MODULEMANAGER_LOG(Verbose, "Module '{:s}' has been unloaded", moduleName);
}
