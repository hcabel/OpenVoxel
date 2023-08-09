#include "UIModule.h"
#include "UIGlobals.h"
#include "UI.h"
#include "Renderer.h"

IMPLEMENT_MODULE(UIModule);

void UIModule::StartupModule()
{
	// glfwInit need to be call in each module that use any glfw function
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	UI::Get().Init();
}

void UIModule::ShutdownModule()
{
	UI::Get().ShutDown();
}
