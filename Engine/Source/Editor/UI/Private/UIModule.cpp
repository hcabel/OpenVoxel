#include "UIModule.h"

#include <GLFW/glfw3.h>

IMPLEMENT_MODULE(UIModule);

void UIModule::StartupModule()
{
	glfwInit();
}

void UIModule::ShutdownModule()
{
	glfwTerminate();
}
