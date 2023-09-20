#include "GameWindow.h"
#include "VulkanContext.h"
#include "CoreGlobals.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GameWindow::GameWindow(AxisSize width, AxisSize height, const char* title)
	: Window(width, height, title)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_WindowPtr = glfwCreateWindow(m_Width, m_Height, title, nullptr, nullptr);

	// Set window icon to the OpenVoxel logo
	GLFWimage icon[1];
	std::string path = Path::GetEngineRootDirectoryPath() + "Resources/OpenVoxelLogo 128x128.png";
	icon[0].pixels = stbi_load(path.c_str(), &icon[0].width, &icon[0].height, nullptr, STBI_rgb_alpha);
	glfwSetWindowIcon(m_WindowPtr, 1, icon);

	// Add vulkan extensions
	{
		// Add GLFW vulkan extensions
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (uint32_t i = 0; i < glfwExtensionCount; i++)
			VulkanContext::Get().AddInstanceExtension(glfwExtensions[i]);

		// Add swapchain extension
		VulkanContext::Get().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		// Add ray tracing Extension
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures;
		rayTracingFeatures.rayTracingPipeline = true;
		VulkanContext::Get().AddDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rayTracingFeatures);

		// Acceleration structure extension
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
		accelerationStructureFeatures.accelerationStructure = true;
		VulkanContext::Get().AddDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelerationStructureFeatures);

		// Required by the Acceleration structure extension
		vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
		bufferDeviceAddressFeatures.bufferDeviceAddress = true;
		VulkanContext::Get().AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &bufferDeviceAddressFeatures);
		VulkanContext::Get().AddDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

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

	m_SceneRenderer = SceneRenderer::Create(m_Swapchain.GetFrameCount());
}

GameWindow::~GameWindow()
{
	m_Swapchain.Destroy();

	glfwDestroyWindow(m_WindowPtr);
}

bool GameWindow::IsClosed() const
{
	return glfwWindowShouldClose(m_WindowPtr);
}

void GameWindow::SetSize(AxisSize width, AxisSize height)
{
	m_Width = width;
	m_Height = height;
	glfwSetWindowSize(m_WindowPtr, width, height);
	m_HasBeenResized = true;
}

void GameWindow::Tick(float deltaTime)
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
		if (m_Width != 0 && m_Height != 0) // Do not resize swapchain if the window is minimized
		{
			OV_LOG(LogTemp, Verbose, "Window has been resized to {:d}x{:d}", m_Width, m_Height);
			m_Swapchain.Resize(m_Width, m_Height);
		}
		m_HasBeenResized = false;
	}
}

void GameWindow::Draw()
{
	const VulkanSwapchainFrame& frame = m_Swapchain.AcquireNextFrame();

	auto cmdBuffer = frame.Begin();
	cmdBuffer.pipelineBarrier( // Make the frame ready for rendering
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::DependencyFlags(),
		{},
		{},
		vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eMemoryRead,
			vk::AccessFlagBits::eShaderWrite,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eGeneral,
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
			frame.GetImage(),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
		)
	);

	m_SceneRenderer.RenderScene(
		SceneRenderer::RenderSceneInfo(
			m_Swapchain.GetCurrentFrameIndex(),
			frame.GetWidth(),
			frame.GetHeight(),
			frame.GetImageView(),
			cmdBuffer
		)
	);

	cmdBuffer.pipelineBarrier( // Make the frame ready for presenting
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eBottomOfPipe,
		vk::DependencyFlags(),
		{},
		{},
		vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eMemoryRead,
			vk::ImageLayout::eGeneral,
			vk::ImageLayout::ePresentSrcKHR,
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
			frame.GetImage(),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
		)
	);
	frame.End();

	m_Swapchain.PresentFrame();
}

void GameWindow::OnTitleUpdate()
{
	glfwSetWindowTitle(m_WindowPtr, m_Title);
}
