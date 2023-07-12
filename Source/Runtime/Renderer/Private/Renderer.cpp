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

	m_Dldi.init(m_VkInstance.Raw(), vkGetInstanceProcAddr, m_VkDevice.Raw(), vkGetDeviceProcAddr);

	vk::CommandPoolCreateInfo commandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);
	m_CommandPool = m_VkDevice.Raw().createCommandPool(commandPoolCreateInfo);

	InitSwapChain();

	CreateRayTracingImage(m_VkSwapChain.GetFrames().back().CommandBuffer);

	m_AccelerationStructure.SetVulkanDevice(&m_VkDevice);
	m_AccelerationStructure.SetDispatchLoaderDynamic(&m_Dldi);
	m_AccelerationStructure.CreateAccelerationStructure(m_VkSwapChain.GetFrames().back().CommandBuffer);

	m_DescriptorSet.SetVulkanDevice(&m_VkDevice);
	m_DescriptorSet.SetVulkanAccelerationStructure(&m_AccelerationStructure);
	m_DescriptorSet.CreateDescriptorSet(m_RayTracingImageView);

	m_Pipeline.SetVulkanDevice(&m_VkDevice);
	m_Pipeline.SetDispatchLoaderDynamic(&m_Dldi);
	m_Pipeline.CreateRayTracingPipeline(m_DescriptorSet);

	m_ShaderBindingTable.SetVulkanDevice(&m_VkDevice);
	m_ShaderBindingTable.SetVulkanRayTracingPipeline(&m_Pipeline);
	m_ShaderBindingTable.SetDispatchLoaderDynamic(&m_Dldi);
	m_ShaderBindingTable.CreateShaderBindingTable();

	for (auto& frame : m_VkSwapChain.GetFrames())
	{
		frame.CommandBuffer.reset();
		frame.CommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		frame.CommandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_Pipeline);

		frame.CommandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			m_Pipeline.GetPipelineLayout(),
			0,
			1,
			&m_DescriptorSet.GetDescriptorSet(),
			0,
			nullptr
		);

		auto rgen = m_ShaderBindingTable.GetRaygenShaderBindingTable();
		auto rmiss = m_ShaderBindingTable.GetMissShaderBindingTable();
		auto rchit = m_ShaderBindingTable.GetHitShaderBindingTable();
		auto rcall = m_ShaderBindingTable.GetCallableShaderBindingTable();
		frame.CommandBuffer.traceRaysKHR(
			rgen, rmiss, rchit, rcall,
			m_VkSwapChain.GetExtent().width,
			m_VkSwapChain.GetExtent().height,
			1,
			m_Dldi
		);

		vk::ImageMemoryBarrier swapchainCopyMemoryBarrier(
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eTransferWrite,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
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
			1, &swapchainCopyMemoryBarrier
		);

		vk::ImageMemoryBarrier rayTraceCopyMemoryBarrier(
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eTransferRead,
			vk::ImageLayout::eGeneral,
			vk::ImageLayout::eTransferSrcOptimal,
			m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
			m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
			m_RayTracingImage,
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
			1, &rayTraceCopyMemoryBarrier
		);

		vk::ImageCopy imageCopy(
			vk::ImageSubresourceLayers(
				vk::ImageAspectFlagBits::eColor,
				0, 0, 1
			),
			vk::Offset3D(0, 0, 0),
			vk::ImageSubresourceLayers(
				vk::ImageAspectFlagBits::eColor,
				0, 0, 1
			),
			vk::Offset3D(0, 0, 0),
			vk::Extent3D(
				m_VkSwapChain.GetExtent().width,
				m_VkSwapChain.GetExtent().height,
				1
			)
		);
		frame.CommandBuffer.copyImage(
			m_RayTracingImage,
			vk::ImageLayout::eTransferSrcOptimal,
			frame.Image,
			vk::ImageLayout::eTransferDstOptimal,
			1, &imageCopy
		);

		vk::ImageMemoryBarrier swapchainPresentMemoryBarrier(
			vk::AccessFlagBits::eTransferWrite,
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::eTransferDstOptimal,
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
			1, &swapchainPresentMemoryBarrier
		);

		vk::ImageMemoryBarrier rayTraceGeneralMemoryBarrier(
			vk::AccessFlagBits::eTransferRead,
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::eTransferSrcOptimal,
			vk::ImageLayout::eGeneral,
			m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
			m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
			m_RayTracingImage,
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
			1, &rayTraceGeneralMemoryBarrier
		);

		frame.CommandBuffer.end();
	}
}

Renderer::~Renderer()
{
	m_VkDevice.Raw().waitIdle();

	m_ShaderBindingTable.DestroyShaderBindingTable();
	m_AccelerationStructure.DestroyAccelerationStructure();

	m_Pipeline.DestroyRayTracingPipeline();

	m_DescriptorSet.DestroyDescriptorSet();

	m_VkSwapChain.DestroySwapChain();

	m_VkDevice.Raw().destroyCommandPool(m_CommandPool);
	m_VkDevice.Raw().destroyImageView(m_RayTracingImageView);
	m_VkDevice.Raw().destroyImage(m_RayTracingImage);
	m_VkDevice.Raw().freeMemory(m_RayTracingImageMemory);

	m_VkDevice.DestroyDevice();

#if OV_DEBUG
	VulkanDebugMessenger::CleanUp(m_VkInstance);
#endif
	m_VkInstance.Raw().destroySurfaceKHR(m_Surface);
	m_VkInstance.DestroyInstance();
}

Renderer* Renderer::Get()
{
	CHECK(s_Instance, "Renderer has never been initialized before, you need to call Renderer::Initialize() first");
	return (s_Instance);
}

Renderer* Renderer::Initialize(GLFWwindow* window)
{
	CHECK(!s_Instance, "Renderer has already been initialized before");
	s_Instance = new Renderer(window);
	return (s_Instance);
}

void Renderer::Shutdown()
{
	CHECK(s_Instance, "Renderer has never been initialized before, you need to call Renderer::Initialize() first");
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

	m_VkSwapChain.SubmitWork(m_CurrentFrameIndex);
	m_VkSwapChain.PresentFrame(m_CurrentFrameIndex);
}

void Renderer::Tick()
{
	glfwPollEvents();

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

	m_VkSwapChain.CreateSwapChain(vk::PresentModeKHR::eMailbox);
}

void Renderer::CreateRayTracingImage(const vk::CommandBuffer& commandBuffer)
{
	vk::ImageCreateInfo rayTracingImageInfo(

		vk::ImageCreateFlags(),
		vk::ImageType::e2D,
		m_VkSwapChain.GetFormat(),
		vk::Extent3D(
			m_VkSwapChain.GetExtent().width,
			m_VkSwapChain.GetExtent().height,
			1
		),
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		vk::ImageLayout::eUndefined
	);

	m_RayTracingImage = m_VkDevice.Raw().createImage(rayTracingImageInfo);

	VulkanMemoryRequirementsExtended rayTracingImageMemoryRequirements = m_VkDevice.FindMemoryRequirement(m_RayTracingImage, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo rayTracingImageMemoryAllocateInfo(
		rayTracingImageMemoryRequirements.size,
		rayTracingImageMemoryRequirements.MemoryTypeIndex
	);

	m_RayTracingImageMemory = m_VkDevice.Raw().allocateMemory(rayTracingImageMemoryAllocateInfo);

	vk::ImageViewCreateInfo rayTracingImageViewInfo(
		vk::ImageViewCreateFlags(),
		m_RayTracingImage,
		vk::ImageViewType::e2D,
		m_VkSwapChain.GetFormat(),
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);

	m_VkDevice.Raw().bindImageMemory(m_RayTracingImage, m_RayTracingImageMemory, 0);

	m_RayTracingImageView = m_VkDevice.Raw().createImageView(rayTracingImageViewInfo);

	commandBuffer.reset();
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	vk::ImageMemoryBarrier rayTraceGeneralMemoryBarrier(
		vk::AccessFlagBits::eNone,
		vk::AccessFlagBits::eNone,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eGeneral,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		m_RayTracingImage,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);
	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eAllCommands,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &rayTraceGeneralMemoryBarrier
	);

	commandBuffer.end();

	vk::Fence fence = m_VkDevice.Raw().createFence(vk::FenceCreateInfo());

	vk::SubmitInfo submitInfo(
		0, nullptr,
		nullptr,
		1, &commandBuffer,
		0, nullptr
	);
	m_VkDevice.GetQueue(VulkanQueueType::Graphic).submit(1, &submitInfo, fence);

	m_VkDevice.Raw().waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
	m_VkDevice.Raw().destroyFence(fence);
}