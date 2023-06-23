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

	OV_LOG(LogVulkan, Verbose, "Swap chain properties: ");
	OV_LOG(LogVulkan, Verbose, "\tFormat: {:s} ({:s})", vk::to_string(m_SurfaceFormat.format), vk::to_string(m_SurfaceFormat.colorSpace));
	OV_LOG(LogVulkan, Verbose, "\tPresent mode: {:s}", vk::to_string(m_PresentMode));
	OV_LOG(LogVulkan, Verbose, "\tExtent: {:d}x{:d}", m_Extent.width, m_Extent.height);

	m_ImageCount = std::min(
		supportProperties.Capabilities.minImageCount + 1,
		supportProperties.Capabilities.maxImageCount
	);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo(
		vk::SwapchainCreateFlagsKHR(),
		*m_Surface,
		m_ImageCount,
		m_SurfaceFormat.format,
		m_SurfaceFormat.colorSpace,
		m_Extent,
		1,
		vk::ImageUsageFlagBits::eTransferDst,
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
		OV_LOG(LogVulkan, Fatal, "Failed to create swap chain: \"{:s}\"", e.what());
	}

	/* Create frames */
	std::vector<vk::Image> images = m_VkDevice->Raw().getSwapchainImagesKHR(m_Swapchain);
	m_ImageCount = static_cast<uint32_t>(images.size());
	m_Frames.reserve(m_ImageCount);
	for (uint32_t i = 0; i < m_ImageCount; ++i)
		m_Frames.push_back(CreateFrame(images[i]));

	m_IsSwapChainCreated = true;
}

void VulkanSwapChainHandler::DestroySwapChain()
{
	CHECK(m_IsSwapChainCreated);

	// destroy every allocated command buffer
	for (auto& frame : m_Frames)
	{
		m_VkDevice->Raw().destroySemaphore(frame.RenderedSemaphore);
		m_VkDevice->Raw().destroySemaphore(frame.AcquiredSemaphore);
		m_VkDevice->Raw().destroyFence(frame.RenderedFence);
		m_VkDevice->Raw().freeCommandBuffers(*m_CommandPool, frame.CommandBuffer);
	}

	m_VkDevice->Raw().destroySwapchainKHR(m_Swapchain);

	m_VkDevice = nullptr;
	m_Surface = nullptr;

	m_IsSwapChainCreated = false;
}

uint8_t VulkanSwapChainHandler::AcquireNextFrameIndex()
{
	CHECK(m_IsSwapChainCreated);

	m_CurrentFrameIndex =
		static_cast <uint8_t>(m_VkDevice->Raw().acquireNextImageKHR(m_Swapchain, UINT64_MAX, m_Frames[m_CurrentSemaphoreIndex].AcquiredSemaphore, nullptr).value);

	// block until the previous frame has finish rendering (not presenting)
	m_VkDevice->Raw().waitForFences(m_Frames[m_CurrentFrameIndex].RenderedFence, VK_TRUE, UINT64_MAX);
	m_VkDevice->Raw().resetFences(m_Frames[m_CurrentFrameIndex].RenderedFence);

	return (m_CurrentFrameIndex);
}

void VulkanSwapChainHandler::SubmitWork(uint8_t frameIndex) const
{
	CHECK(m_IsSwapChainCreated);

	vk::PipelineStageFlags stageToWait = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo infoToSubmit(
		1, &m_Frames[m_CurrentSemaphoreIndex].AcquiredSemaphore,
		&stageToWait,
		1, &m_Frames[frameIndex].CommandBuffer,
		1, &m_Frames[m_CurrentSemaphoreIndex].RenderedSemaphore
	);

	m_VkDevice->GetQueue(VulkanQueueType::Graphic).submit(infoToSubmit, m_Frames[m_CurrentSemaphoreIndex].RenderedFence);
}

void VulkanSwapChainHandler::PresentFrame(uint8_t frameIndex)
{
	CHECK(m_IsSwapChainCreated);

	uint32_t frameIndex32bit = frameIndex;
	vk::Result result;
	vk::PresentInfoKHR presentInfos(
		1, &m_Frames[m_CurrentSemaphoreIndex].RenderedSemaphore,
		1, &m_Swapchain,
		&frameIndex32bit,
		&result
	);
	m_VkDevice->GetQueue(VulkanQueueType::Graphic).presentKHR(presentInfos);
	m_CurrentSemaphoreIndex = (m_CurrentSemaphoreIndex + 1) % m_ImageCount;
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

	/* Create semaphores */
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.flags = vk::SemaphoreCreateFlags();
	newFrame.AcquiredSemaphore = m_VkDevice->Raw().createSemaphore(semaphoreCreateInfo);
	newFrame.RenderedSemaphore = m_VkDevice->Raw().createSemaphore(semaphoreCreateInfo);

	/* Create fence */
	vk::FenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	newFrame.RenderedFence = m_VkDevice->Raw().createFence(fenceCreateInfo);

	return (newFrame);
}