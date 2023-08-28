#include "GlfwWindow.h"
#include "VulkanContext.h"

#include "CoreGlobals.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

GlfwWindow::GlfwWindow(AxisSize width, AxisSize height, const char* title)
	: Window(width, height, title)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_WindowPtr = glfwCreateWindow(m_Width, m_Height, title, nullptr, nullptr);

	// Get GLFW vulkan extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
		VulkanContext::Get().AddInstanceExtension(glfwExtensions[i]);

	// Add swapchain extension
	VulkanContext::Get().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// Create the vulkan instance in 1.3.224
	// It's a static version, because I want to verify that the everything works before upgrading
	if (VulkanContext::Get().CreateInstance(1, 3, 224) == false)
		return;

	// Get window surface
	vk::SurfaceKHR surface;
	if (glfwCreateWindowSurface(VulkanContext::GetInstance(), m_WindowPtr, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)) != VK_SUCCESS)
		return;

	if (VulkanContext::Get().CreateDevice(surface) == false)
		return;

	// Create swapchain
	m_Swapchain = VulkanSwapchain(vk::PresentModeKHR::eMailbox, surface, m_Width, m_Height);
}

GlfwWindow::~GlfwWindow()
{
	m_Swapchain.Destroy();

	glfwDestroyWindow(m_WindowPtr);
}

bool GlfwWindow::IsClosed() const
{
	return glfwWindowShouldClose(m_WindowPtr);
}

void GlfwWindow::SetSize(AxisSize width, AxisSize height)
{
	m_Width = width;
	m_Height = height;
	glfwSetWindowSize(m_WindowPtr, width, height);
	m_HasBeenResized = true;
}

void GlfwWindow::Tick(float deltaTime)
{
	glfwPollEvents();

	// Get window size if it has been resized
	int width, height;
	glfwGetWindowSize(m_WindowPtr, &width, &height);

	if (width != m_Width || height != m_Height)
	{
		m_Width = width;
		m_Height = height;
		m_HasBeenResized = true;
	}

	if (m_HasBeenResized)
	{
		OV_LOG(LogTemp, Verbose, "Window has been resized to {:d}x{:d}", m_Width, m_Height);
		m_Swapchain.Resize(m_Width, m_Height);
		m_HasBeenResized = false;
	}
}

void GlfwWindow::Draw()
{
	const VulkanSwapchainFrame& frame = m_Swapchain.AcquireNextFrame();

	frame.Begin();

	// TODO: Ask to draw the scene on the frame

	frame.End();

	m_Swapchain.PresentFrame();
}

void GlfwWindow::OnTitleUpdate()
{
	glfwSetWindowTitle(m_WindowPtr, m_Title);
}
