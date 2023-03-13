#include "MainWindow.h"

DEFINE_LOG_CATEGORY(GlfwLog)

MainWindow::MainWindow(uint16_t width, uint16_t height, const char* title)
{
	auto window = CreateWindow(width, height, title);
	if (window.has_value() == false)
		OV_LOG(Fatal, GlfwLog, "Failed to create window");
	m_Window = window.value();

	auto vkInstance = Vulkan::CreateInstance();
	if (vkInstance.has_value() == false)
		OV_LOG(Fatal, GlfwLog, "Failed to create Vulkan instance");
	m_VkInstance = vkInstance.value();

#if OV_DEBUG
	m_DebugMessenger = Vulkan::SetupDebugMessenger(m_VkInstance);
#endif

	auto surface = Vulkan::CreateSurface(m_VkInstance, m_Window);
	if (surface.has_value() == false)
		OV_LOG(Fatal, GlfwLog, "Failed to create Vulkan surface");
	m_Surface = surface.value();

	auto device = Vulkan::CreateDevice(m_VkInstance, m_Surface);
	if (device.has_value() == false)
		OV_LOG(Fatal, GlfwLog, "Failed to create Vulkan device");
	m_Device = device.value();

	auto swapChain = Vulkan::CreateSwapChain(m_VkInstance, m_Device, m_Surface, width, height);
	if (swapChain.has_value() == false)
		OV_LOG(Fatal, GlfwLog, "Failed to create Vulkan SwapChain");
	m_SwapChain = swapChain.value();
}

MainWindow::~MainWindow()
{
	/* Destroy device */
	for (auto& frame : m_SwapChain.Frames)
		m_Device.Logical.destroyImageView(frame.View);
	m_Device.Logical.destroySwapchainKHR(m_SwapChain.SwapChain);
	m_Device.Logical.destroy();

	/* Destroy instance */
#if OV_DEBUG
	m_VkInstance.destroyDebugUtilsMessengerEXT(m_DebugMessenger.Messenger, nullptr, m_DebugMessenger.Dldi);
#endif
	m_VkInstance.destroySurfaceKHR(m_Surface);
	m_VkInstance.destroy();

	/* Destroy glfw */
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

std::optional<GLFWwindow*> MainWindow::CreateWindow(uint16_t width, uint16_t height, const char* title) const noexcept
{
	glfwSetErrorCallback([](int error, const char *description) {
		OV_LOG(Error, GlfwLog, "%d = %s", error, description);
	});

	if (glfwInit() == GLFW_FALSE)
	{
		OV_LOG(Error, GlfwLog, "Failed to initialize GLFW");
		return {};
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: make re-sizable

	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		OV_LOG(Error, GlfwLog, "Failed to create GLFW window");
		return {};
	}
	return (window);
}