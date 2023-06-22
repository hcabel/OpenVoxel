#pragma once

#include "Renderer_API.h"
#include "Module.h"

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
