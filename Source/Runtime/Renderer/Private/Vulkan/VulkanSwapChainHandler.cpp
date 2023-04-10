#include "Vulkan/VulkanSwapChainHandler.h"
#include "Vulkan/VulkanDeviceHandler.h"

VulkanSwapChainHandler::VulkanSwapChainHandler(const VulkanDeviceHandler* vkDevice, const vk::SurfaceKHR* surface)
	: m_VkDevice(vkDevice), m_Surface(surface)
{}

void VulkanSwapChainHandler::CreateSwapChain(vk::PresentModeKHR preferredPresentMode)
{
	CHECK(m_VkDevice && m_Surface);

	VulkanSwapChainSupportProperties supportProperties = RequestSwapchainProperties();

	m_SurfaceFormat = SelectSurfaceFormat(supportProperties.Formats);
	m_PresentMode = SelectPresentMode(supportProperties.PresentModes, preferredPresentMode);
	m_Extent = SelectExtent(supportProperties.Capabilities);

	OV_LOG(Verbose, LogVulkan, "Swap chain properties: ");
	OV_LOG(Verbose, LogVulkan, "\tFormat: {:s} ({:s})", vk::to_string(m_SurfaceFormat.format), vk::to_string(m_SurfaceFormat.colorSpace));
	OV_LOG(Verbose, LogVulkan, "\tPresent mode: {:s}", vk::to_string(m_PresentMode));
	OV_LOG(Verbose, LogVulkan, "\tExtent: {:d}x{:d}", m_Extent.width, m_Extent.height);

	uint32_t imageCount = std::min(
		supportProperties.Capabilities.minImageCount + 1,
		supportProperties.Capabilities.maxImageCount
	);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo(
		vk::SwapchainCreateFlagsKHR(),
		*m_Surface,
		imageCount,
		m_SurfaceFormat.format,
		m_SurfaceFormat.colorSpace,
		m_Extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		supportProperties.Capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		m_PresentMode,
		VK_TRUE
	);

	try {
		m_Swapchain = m_VkDevice->Raw().createSwapchainKHR(swapChainCreateInfo);
	}
	catch (vk::SystemError& e)
	{
		OV_LOG(Fatal, LogVulkan, "Failed to create swap chain: \"{:s}\"", e.what());
	}

	/* Create frames */
	std::vector<vk::Image> images = m_VkDevice->Raw().getSwapchainImagesKHR(m_Swapchain);
	imageCount = static_cast<uint32_t>(images.size());
	m_Frames.reserve(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
		m_Frames.push_back(CreateFrame(images[i]));

	/* Create semaphores */
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.flags = vk::SemaphoreCreateFlags();
	m_FrameAcquiredSemaphore = m_VkDevice->Raw().createSemaphore(semaphoreCreateInfo);
	m_FrameRenderedSemaphore = m_VkDevice->Raw().createSemaphore(semaphoreCreateInfo);

	/* Create fence */
	vk::FenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	m_FrameRenderedFence = m_VkDevice->Raw().createFence(fenceCreateInfo);

	m_IsSwapChainCreated = true;
}

void VulkanSwapChainHandler::DestroySwapChain()
{
	CHECK(m_IsSwapChainCreated);

	m_VkDevice->Raw().destroySemaphore(m_FrameRenderedSemaphore);
	m_VkDevice->Raw().destroySemaphore(m_FrameAcquiredSemaphore);
	m_VkDevice->Raw().destroyFence(m_FrameRenderedFence);

	// destroy every allocated command buffer
	for (auto& frame : m_Frames)
		m_VkDevice->Raw().freeCommandBuffers(*m_CommandPool, frame.CommandBuffer);

	m_VkDevice->Raw().destroySwapchainKHR(m_Swapchain);

	m_VkDevice = nullptr;
	m_Surface = nullptr;

	m_IsSwapChainCreated = false;
}

uint8_t VulkanSwapChainHandler::AcquireNextFrameIndex() const
{
	CHECK(m_IsSwapChainCreated);

	// block until the previous frame has finish rendering (not presenting)
	m_VkDevice->Raw().waitForFences(m_FrameRenderedFence, VK_TRUE, UINT64_MAX);
	m_VkDevice->Raw().resetFences(m_FrameRenderedFence);

	uint8_t imageIndex =
		static_cast <uint8_t>(m_VkDevice->Raw().acquireNextImageKHR(m_Swapchain, UINT64_MAX, m_FrameAcquiredSemaphore, nullptr).value);
	return (imageIndex);
}

void VulkanSwapChainHandler::SubmitWork(uint8_t frameIndex) const
{
	CHECK(m_IsSwapChainCreated);

	vk::PipelineStageFlags stageToWait = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo infoToSubmit(
		1, &m_FrameAcquiredSemaphore,
		&stageToWait,
		1, &m_Frames[frameIndex].CommandBuffer,
		1, &m_FrameRenderedSemaphore
	);

	m_VkDevice->GetQueue(VulkanQueueType::Graphic).submit(infoToSubmit, m_FrameRenderedFence);
}

void VulkanSwapChainHandler::PresentFrame(uint8_t frameIndex) const
{
	CHECK(m_IsSwapChainCreated);

	uint32_t frameIndex32bit = frameIndex;
	vk::Result result;
	vk::PresentInfoKHR presentInfos(
		1, &m_FrameRenderedSemaphore,
		1, &m_Swapchain,
		&frameIndex32bit,
		&result
	);
	m_VkDevice->GetQueue(VulkanQueueType::Present).presentKHR(presentInfos);
}

VulkanSwapChainSupportProperties VulkanSwapChainHandler::RequestSwapchainProperties() const
{
	VulkanSwapChainSupportProperties properties;

	properties.Capabilities = m_VkDevice->GetPhysicalDevice().getSurfaceCapabilitiesKHR(*m_Surface);
	properties.Formats = m_VkDevice->GetPhysicalDevice().getSurfaceFormatsKHR(*m_Surface);
	properties.PresentModes = m_VkDevice->GetPhysicalDevice().getSurfacePresentModesKHR(*m_Surface);

	return (properties);
}

vk::SurfaceFormatKHR VulkanSwapChainHandler::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> availableFormats, vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace) const
{
	CHECK(availableFormats.empty() == false);

	for (const auto& format : availableFormats)
	{
		if (format.format == preferredFormat && format.colorSpace == preferredColorSpace)
			return (format);
	}
	return (availableFormats[0]);
}

vk::PresentModeKHR VulkanSwapChainHandler::SelectPresentMode(std::vector<vk::PresentModeKHR> availablePresentModeList, vk::PresentModeKHR preferredPresentMode) const
{
	CHECK(availablePresentModeList.empty() == false);

	for (const auto& presentMode : availablePresentModeList)
	{
		if (presentMode == preferredPresentMode)
			return presentMode;
	}
	return (vk::PresentModeKHR::eFifo); // Guarantied to be available by vulkan
}

vk::Extent2D VulkanSwapChainHandler::SelectExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent == UINT32_MAX)
	{
		// Calculate the median of the min and max extent
		return (vk::Extent2D(
			(capabilities.minImageExtent.width + capabilities.maxImageExtent.width) / 2,
			(capabilities.minImageExtent.height + capabilities.maxImageExtent.height) / 2
		));
	}
	return (capabilities.currentExtent);
}

VulkanSwapChainFrame VulkanSwapChainHandler::CreateFrame(const vk::Image& image)
{
	CHECK(m_CommandPool);

	VulkanSwapChainFrame newFrame;

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
		*m_CommandPool,
		vk::CommandBufferLevel::ePrimary,
		1
	);

	newFrame.CommandBuffer = m_VkDevice->Raw().allocateCommandBuffers(commandBufferAllocateInfo)[0];
	newFrame.Image = image;

	return (newFrame);
}

