#include "Renderer.h"
#include "Vulkan/VulkanDebugMessenger.h"
#include "HAL/PlatformTime.h"

#include <format>

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

	auto randomFrame = m_VkSwapChain.GetFrames().back();
	m_AccelerationStructure.SetVulkanDevice(&m_VkDevice);
	m_AccelerationStructure.SetDispatchLoaderDynamic(&m_Dldi);
	m_AccelerationStructure.CreateAccelerationStructure(randomFrame.CommandBuffer);

	m_PipelineCache = m_VkDevice.Raw().createPipelineCache(vk::PipelineCacheCreateInfo());

	m_Pipeline.SetVulkanDevice(&m_VkDevice);
	m_Pipeline.SetDispatchLoaderDynamic(&m_Dldi);
	m_Pipeline.CreateRayTracingPipeline(randomFrame.DescriptorSetLayout, m_PipelineCache);

	m_ShaderBindingTable.SetVulkanDevice(&m_VkDevice);
	m_ShaderBindingTable.SetVulkanRayTracingPipeline(&m_Pipeline);
	m_ShaderBindingTable.SetDispatchLoaderDynamic(&m_Dldi);
	m_ShaderBindingTable.CreateShaderBindingTable();

	// Record each command buffer and set their descriptor set
	for (auto& frame : m_VkSwapChain.GetFrames())
	{
		std::vector<vk::WriteDescriptorSet> descriptorWrites;

		// Get TLAS and write it on the descriptor set (layout location 0)
		vk::AccelerationStructureKHR accelerationStructure = m_AccelerationStructure.GetTlas();
		vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo(
			1, &accelerationStructure
		);
		descriptorWrites.push_back(
			vk::WriteDescriptorSet(
				frame.DescriptorSet,
				0,
				0,
				1, vk::DescriptorType::eAccelerationStructureKHR,
				nullptr,
				nullptr,
				nullptr,
				&accelerationStructureInfo
			)
		);

		// Write the current image view on the descriptor set (layout location 1)
		vk::DescriptorImageInfo imageInfo(
			vk::Sampler(),
			frame.ImageView,
			vk::ImageLayout::eGeneral
		);
		descriptorWrites.push_back(
			vk::WriteDescriptorSet(
				frame.DescriptorSet,
				1,
				0,
				1, vk::DescriptorType::eStorageImage,
				&imageInfo,
				nullptr,
				nullptr
			)
		);

		// Push all the update to the descriptor set (so it's actually updated)
		m_VkDevice.Raw().updateDescriptorSets(
			descriptorWrites.size(), descriptorWrites.data(),
			0,
			nullptr
		);

		// Buffer
		frame.CommandBuffer.reset();
		frame.CommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

		// Bind raytracing pipeline
		frame.CommandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_Pipeline);

		// Bind the descriptor set (allow shader to access the data stored in the descriptor set)
		frame.CommandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			m_Pipeline.GetPipelineLayout(),
			0,
			1, &frame.DescriptorSet,
			0,
			nullptr
		);

		ShaderWriteBarrier(frame.CommandBuffer, frame.Image, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eUndefined);
		TraceRays(frame.CommandBuffer);
		PresentBarrier(frame.CommandBuffer, frame.Image, vk::AccessFlagBits::eShaderWrite, vk::ImageLayout::eGeneral);

		frame.CommandBuffer.end();
	}
}

Renderer::~Renderer()
{
	m_VkDevice.Raw().waitIdle();

	m_ShaderBindingTable.DestroyShaderBindingTable();
	m_AccelerationStructure.DestroyAccelerationStructure();

	m_VkDevice.Raw().destroyPipelineCache(m_PipelineCache);
	m_Pipeline.DestroyRayTracingPipeline();

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

Renderer& Renderer::Get()
{
	return (*s_Instance);
}

Renderer& Renderer::Initialize(GLFWwindow* window)
{
	s_Instance = new Renderer(window);
	return (*s_Instance);
}

void Renderer::Shutdown()
{
	delete s_Instance;
	s_Instance = nullptr;
}

void Renderer::PrepareNewFrame()
{
	m_CurrentFrameIndex = m_VkSwapChain.AcquireNextFrameIndex();
}

void Renderer::RenderNewFrame()
{
	m_VkSwapChain.SubmitWork(GetCurrentFrameIndex());
	m_VkSwapChain.PresentFrame(GetCurrentFrameIndex());
}

void Renderer::Tick()
{
	glfwPollEvents();

	// Update title to show fps
	const uint16_t fpsCount = (uint16_t)std::round(1.0f / PlatformTime::GetTimeStep());
	GLFWwindow* glfwWindow = RendererModule::GetWindow();
	glfwSetWindowTitle(glfwWindow, std::format("OpenVoxel - {:.2f}ms = {:d}fps", PlatformTime::GetTimeStep() * 100.0f, fpsCount).c_str());
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

	m_VkSwapChain.CreateSwapChain(vk::PresentModeKHR::eFifo);
}

void Renderer::ShaderWriteBarrier(const vk::CommandBuffer& cmdBuffer, const vk::Image& image, vk::AccessFlagBits previousAccessFlag, vk::ImageLayout previousImageLayout)
{
	vk::ImageMemoryBarrier shaderWriteBarrier(
		previousAccessFlag,
		vk::AccessFlagBits::eShaderWrite,
		previousImageLayout,
		vk::ImageLayout::eGeneral,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		image,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);
	cmdBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &shaderWriteBarrier
	);
}

void Renderer::TraceRays(const vk::CommandBuffer& cmdBuffer)
{
	auto rgen = m_ShaderBindingTable.GetRaygenShaderBindingTable();
	auto rmiss = m_ShaderBindingTable.GetMissShaderBindingTable();
	auto rchit = m_ShaderBindingTable.GetHitShaderBindingTable();
	auto rcall = m_ShaderBindingTable.GetCallableShaderBindingTable();
	cmdBuffer.traceRaysKHR(
		rgen, rmiss, rchit, rcall,
		m_VkSwapChain.GetExtent().width,
		m_VkSwapChain.GetExtent().height,
		1,
		m_Dldi
	);
}

void Renderer::PresentBarrier(const vk::CommandBuffer cmdBuffer, const vk::Image& image, vk::AccessFlagBits previousAccessFlag, vk::ImageLayout previousImageLayout)
{
	vk::ImageMemoryBarrier presentBarrier(
		previousAccessFlag,
		vk::AccessFlagBits::eMemoryRead,
		previousImageLayout,
		vk::ImageLayout::ePresentSrcKHR,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		m_VkDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		image,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);

	cmdBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eBottomOfPipe, // No specific stage needs to start after the transition
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &presentBarrier
	);
}
