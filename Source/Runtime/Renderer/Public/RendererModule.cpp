#include "RendererModule.h"
#include "Renderer.h"

#include <GLFW/glfw3.h>

RENDERER_API DEFINE_MODULE(RendererModule);

GLFWwindow* RendererModule::s_Window = nullptr;

void RendererModule::StartupModule()
{
	// Create glfwWindow
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO
	s_Window = glfwCreateWindow(1280, 720, "OpenVoxel", nullptr, nullptr);

	Renderer::Initialize(s_Window);
}

void RendererModule::ShutdownModule()
{
	Renderer::Shutdown();
	s_Window = nullptr;
}
