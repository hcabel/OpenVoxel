#pragma once

#include "Core_API.h"
#include "MacrosHelper.h"

#include <inttypes.h>

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
 * Module is the base class for every module.
 */
class CORE_API Module
{

public:
	Module() = default;

	Module(const Module&) = delete;
	Module& operator=(const Module&) = delete;

public:
	/** Called when the module is loaded */
	virtual void StartupModule() = 0;
	/** Called when unloading the module */
	virtual void ShutdownModule() = 0;
};

// This macro is used to implement the module
// By implementing the module, it allow the module manager to load it
#define IMPLEMENT_MODULE(ModuleClass) \
	extern "C" OV_DLL_EXPORT Module* CreateModule() \
	{ \
		return new ModuleClass(); \
	}
