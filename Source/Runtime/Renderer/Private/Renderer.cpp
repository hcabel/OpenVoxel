#include "Renderer.h"
#include "Vulkan/VulkanDebugMessenger.h"

Renderer* Renderer::s_Instance = nullptr;

Renderer::Renderer(GLFWwindow* window)
{
	InitVulkanInstance();
#if OV_DEBUG
	VulkanDebugMessenger::Initialize(m_VkInstance);
#endif
	InitSurfaceKHR(window);
	InitVulkanDevice();

	vk::CommandPoolCreateInfo commandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);
	m_CommandPool = m_VkDevice.Raw().createCommandPool(commandPoolCreateInfo);

	InitSwapChain();
}

Renderer::~Renderer()
{
	m_VkDevice.Raw().waitIdle();

	m_VkSwapChain.DestroySwapChain();

	m_VkDevice.Raw().destroyCommandPool(m_CommandPool);
	m_VkDevice.DestroyDevice();

#if OV_DEBUG
	VulkanDebugMessenger::CleanUp(m_VkInstance);
#endif
	m_VkInstance.Raw().destroySurfaceKHR(m_Surface);
	m_VkInstance.DestroyInstance();
}

Renderer* Renderer::Get()
{
	CHECKF(s_Instance, "Renderer has never been initialized before, you need to call Renderer::Initialize() first");
	return (s_Instance);
}

Renderer* Renderer::Initialize(GLFWwindow* window)
{
	CHECKF(!s_Instance, "Renderer has already been initialized before");
	s_Instance = new Renderer(window);
	return (s_Instance);
}

void Renderer::Shutdown()
{
	CHECKF(s_Instance, "Renderer has never been initialized before, you need to call Renderer::Initialize() first");
	delete s_Instance;
	s_Instance = nullptr;
}

void Renderer::PrepareNewFrame()
{
	m_CurrentFrameIndex = m_VkSwapChain.AcquireNextFrameIndex();
}

void Renderer::RenderNewFrame()
{
	const VulkanSwapChainFrame frame = m_VkSwapChain.GetFrame(m_CurrentFrameIndex);
	frame.CommandBuffer.reset();

	// Record command Buffer
	vk::CommandBufferBeginInfo beginInfo(
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	);
	frame.CommandBuffer.begin(beginInfo);

	vk::ImageMemoryBarrier imageMemoryBarrier(
		vk::AccessFlagBits::eNone,
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		frame.Image,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);
	frame.CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eAllCommands,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier
	);

	frame.CommandBuffer.end();

	m_VkSwapChain.SubmitWork(m_CurrentFrameIndex);
	m_VkSwapChain.PresentFrame(m_CurrentFrameIndex);
}

void Renderer::InitVulkanInstance()
{
	// Add extensions that are required for the use of GLFW
	uint32_t glfwRequiredExtensionsCount = 0;
	const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionsCount);
	for (uint32_t i = 0; i < glfwRequiredExtensionsCount; i++)
		m_VkInstance.AddExtension(glfwRequiredExtensions[i]);

#if OV_DEBUG
	m_VkInstance.AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	m_VkInstance.AddLayer("VK_LAYER_KHRONOS_validation");
#endif

	m_VkInstance.CreateInstance("OpenVoxel", 0);
}

void Renderer::InitSurfaceKHR(GLFWwindow* window)
{
	VkSurfaceKHR surface;
	VkResult result = glfwCreateWindowSurface(m_VkInstance, window, nullptr, &surface);
	CHECK_VULKAN_RESULT(vk::Result(result), "Unable to create window surface");

	m_Surface = surface;
}

void Renderer::InitVulkanDevice()
{
	m_VkDevice.SetVulkanInstance(&m_VkInstance);

	/* Extensions */

	m_VkDevice.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Allow to present on window

	// Ray tracing pipeline
	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = {};
	rtPipelineFeatures.rayTracingPipeline = VK_TRUE;
	m_VkDevice.AddExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rtPipelineFeatures);

	// Build acceleration structure
	vk::PhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = {};
	asFeatures.accelerationStructure = VK_TRUE;
	m_VkDevice.AddExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &asFeatures);

	// Required by VK_KHR_acceleration_structure
	vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures = {};
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	m_VkDevice.AddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &bufferDeviceAddressFeatures);
	m_VkDevice.AddExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	// m_VkDevice.AddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

	// Required for VK_KHR_ray_tracing_pipeline
	m_VkDevice.AddExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

	// Required by VK_KHR_spirv_1_4
	m_VkDevice.AddExtension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);

	/* Layers */

#if OV_DEBUG
	m_VkDevice.AddLayer("VK_LAYER_KHRONOS_validation");
#endif

	m_VkDevice.CreateDevice(m_Surface);
}

void Renderer::InitSwapChain()
{
	m_VkSwapChain.SetVulkanDevice(&m_VkDevice);
	m_VkSwapChain.SetSurface(&m_Surface);
	m_VkSwapChain.SetCommandPool(&m_CommandPool);

	m_VkSwapChain.CreateSwapChain();
}