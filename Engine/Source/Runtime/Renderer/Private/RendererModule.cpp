#include "RendererModule.h"
#include "Renderer.h"

#include <GLFW/glfw3.h>

IMPLEMENT_MODULE(RendererModule);


void RendererModule::StartupModule()
{
	Renderer::Create();
}

void RendererModule::ShutdownModule()
{
	Renderer::Shutdown();
}
