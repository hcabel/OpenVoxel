#include "OVModuleManager.h"
#include "HAL/File.h"
#include "HAL/DLLFile.h"
#include "Path.h"

DEFINE_LOG_CATEGORY(ModuleManagerLog);

void OVModuleManager::LoadModule(const char* moduleName)
{
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
	module->StartupModule();

	MODULEMANAGER_LOG(Verbose, "Module '{:s}' has been loaded", moduleName);
}
