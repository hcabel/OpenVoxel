#include "RendererModule.h"
#include "OVModuleManager.h"

#include <GLFW/glfw3.h>

IMPLEMENT_MODULE(RendererModule);

void RendererModule::StartupModule()
{
	glfwInit();

	OVModuleManager::Get().Load("Vulkan");
}

void RendererModule::ShutdownModule()
{
	OVModuleManager::Get().Unload("Vulkan");

	glfwTerminate();
}
