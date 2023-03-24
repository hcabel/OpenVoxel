#include "MainWindow.h"
#include "Vulkan/VulkanPipeline.h"

DEFINE_LOG_CATEGORY(GlfwLog)

MainWindow::MainWindow(uint16_t width, uint16_t height, const char* title)
{
	/* GLFW */
	auto window = CreateWindow(width, height, title);
	if (window.has_value() == false)
		OV_LOG(Fatal, GlfwLog, "Failed to create window");
	m_Window = window.value();

	/* VULKAN SETUP */
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

	/* VULKAN PIPELINE */
	VulkanPipeline::CreatePipeline(m_Device);

	/* Create commands pool */
	vk::CommandPoolCreateInfo commandPoolCreateInfo(
		vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		m_Device.Queues[QueueType::Graphics].FamilyIndex
	);
	m_CommandPool = m_Device.Logical.createCommandPool(commandPoolCreateInfo);

	m_Swapchain = std::make_unique<VulkanSwapchain>(m_Device, m_Surface, m_CommandPool, vk::PresentModeKHR::eMailbox);
}

MainWindow::~MainWindow()
{
	/* Destroy device */
	m_Swapchain.reset();
	m_Device.Logical.destroyCommandPool(m_CommandPool);
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

void MainWindow::NewFrame()
{
	VulkanSwapchain::Frame frame = m_Swapchain->GetNextFrame();

	m_Swapchain->PresentFrame(frame);
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