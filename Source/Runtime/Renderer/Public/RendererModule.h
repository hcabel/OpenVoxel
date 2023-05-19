#pragma once

#include "MacrosHelper.h"
#include "Module.h"

#if OV_BUILD_DLL
# define RENDERER_API OV_DLL_EXPORT
#else
# define RENDERER_API OV_DLL_IMPORT
#endif

struct GLFWwindow;

class RENDERER_API RendererModule : public AModule
{

public:
	void StartupModule() override;
	void ShutdownModule() override;

public:
	__forceinline static GLFWwindow* GetWindow() { return s_Window; }

private:
	static GLFWwindow* s_Window;
};

REGISTER_MODULE(RendererModule);
