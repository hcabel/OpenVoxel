#include "Vulkan/VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(const Vulkan::DeviceBundle& device, const vk::SurfaceKHR& surface, const vk::CommandPool& commandPool, vk::PresentModeKHR preferredPresentMode)
	: m_Device(device), m_CommandPool(commandPool)
{
	SupportProperties supportProperties = RequestSwapchainProperties(surface);

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
		surface,
		imageCount,
		m_SurfaceFormat.format,
		m_SurfaceFormat.colorSpace,
		m_Extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment
	);

	// If the graphics and present queues are from different queue families, we either have to explicitly transfer ownership of images between the queues, or we have to create the swapchain with imageSharingMode as vk::SharingMode::eConcurrent
	if (m_Device.Queues[QueueType::Graphics].FamilyIndex != m_Device.Queues[QueueType::Present].FamilyIndex)
	{
		uint32_t queueFamilyIndices[] = {
			m_Device.Queues[QueueType::Graphics].FamilyIndex,
			m_Device.Queues[QueueType::Present].FamilyIndex
		};
		swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
		swapChainCreateInfo.setQueueFamilyIndexCount(2);
		swapChainCreateInfo.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else
	{
		swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
		swapChainCreateInfo.setQueueFamilyIndexCount(0);
		swapChainCreateInfo.setPQueueFamilyIndices(nullptr);
	}

	swapChainCreateInfo.preTransform = supportProperties.Capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainCreateInfo.presentMode = m_PresentMode;
	swapChainCreateInfo.clipped = VK_TRUE;

	try {
		m_Swapchain = device.Logical.createSwapchainKHR(swapChainCreateInfo);
	}
	catch (vk::SystemError &e)
	{
		OV_LOG(Fatal, LogVulkan, "Failed to create swap chain: \"{:s}\"", e.what());
	}

	/* Create frames */
	std::vector<vk::Image> images = device.Logical.getSwapchainImagesKHR(m_Swapchain);
	imageCount = static_cast<uint32_t>(images.size());
	m_Frames.reserve(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
		m_Frames.push_back(CreateFrame(i, images[i], commandPool));

	/* Create semaphores */
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.flags = vk::SemaphoreCreateFlags();
	m_ImageAvailableSemaphore = m_Device.Logical.createSemaphore(semaphoreCreateInfo);
	m_RenderFinishedSemaphore = m_Device.Logical.createSemaphore(semaphoreCreateInfo);

	/* Create fence */
	vk::FenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	m_RenderFinishedFence = m_Device.Logical.createFence(fenceCreateInfo);
}

VulkanSwapchain::~VulkanSwapchain()
{
	for (Frame& frame : m_Frames)
	{
		m_Device.Logical.destroyImageView(frame.View);
		m_Device.Logical.freeCommandBuffers(m_CommandPool, 1ui32, &frame.CommandBuffer);
	}

	m_Device.Logical.destroySemaphore(m_ImageAvailableSemaphore);
	m_Device.Logical.destroySemaphore(m_RenderFinishedSemaphore);
	m_Device.Logical.destroyFence(m_RenderFinishedFence);
	m_Device.Logical.destroySwapchainKHR(m_Swapchain);
}

VulkanSwapchain::Frame& VulkanSwapchain::GetNextFrame()
{
	vk::Result lastVkCallResult;

	// Wait for the previous frame to finish his rendering
	lastVkCallResult = m_Device.Logical.waitForFences(1, &m_RenderFinishedFence, VK_TRUE, UINT64_MAX);
	OV_LOG_IF(lastVkCallResult != vk::Result::eSuccess, Warning, LogVulkan, "Failed to wait for fence: \"{:s}\"", vk::to_string(lastVkCallResult))
	// Lock the fence behind to prevent the next frame to start rendering before the current one is finished
	lastVkCallResult = m_Device.Logical.resetFences(1, &m_RenderFinishedFence);
	OV_LOG_IF(lastVkCallResult != vk::Result::eSuccess, Warning, LogVulkan, "Failed to reset fence: \"{:s}\"", vk::to_string(lastVkCallResult));

	// Get next image and signal m_ImageAvailable Semaphore when done
	lastVkCallResult = m_Device.Logical.acquireNextImageKHR(m_Swapchain, UINT32_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &m_CurrentFrameIndex);
	OV_LOG_IF(lastVkCallResult != vk::Result::eSuccess, Warning, LogVulkan, "Failed to acquire next image: \"{:s}\"", vk::to_string(lastVkCallResult));

	return (m_Frames[m_CurrentFrameIndex]);
}

void VulkanSwapchain::PresentFrame(const Frame& frame)
{
	vk::Result vkRenderResult;
	vk::PresentInfoKHR presentInfos(
		1ui32, &m_RenderFinishedSemaphore,
		1ui32, &m_Swapchain,
		&frame.Index,
		&vkRenderResult
	);
	m_Device.Queues[QueueType::Present].Queue.presentKHR(presentInfos);
	OV_LOG_IF(vkRenderResult != vk::Result::eSuccess, Warning, LogVulkan, "Failed to present frame: \"{:s}\"", vk::to_string(vkRenderResult));
}

VulkanSwapchain::SupportProperties VulkanSwapchain::RequestSwapchainProperties(const vk::SurfaceKHR& surface)
{
	SupportProperties properties;

	properties.Capabilities = m_Device.Physical.getSurfaceCapabilitiesKHR(surface);
	properties.Formats = m_Device.Physical.getSurfaceFormatsKHR(surface);
	properties.PresentModes = m_Device.Physical.getSurfacePresentModesKHR(surface);

	return (properties);
}

vk::SurfaceFormatKHR VulkanSwapchain::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> availableFormats, vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace) const
{
	for (const auto& format : availableFormats)
	{
		if (format.format == preferredFormat && format.colorSpace == preferredColorSpace)
			return (format);
	}
	return (availableFormats[0]);
}

vk::PresentModeKHR VulkanSwapchain::SelectPresentMode(std::vector<vk::PresentModeKHR> availablePresentModeList, vk::PresentModeKHR preferredPresentMode) const
{
	for (const auto& presentMode : availablePresentModeList)
	{
		if (presentMode == preferredPresentMode)
			return presentMode;
	}
	return (vk::PresentModeKHR::eFifo); // Guarantied to be available
}

vk::Extent2D VulkanSwapchain::SelectExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
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

VulkanSwapchain::Frame VulkanSwapchain::CreateFrame(uint32_t frameIndex, vk::Image& image, const vk::CommandPool& commandPool)
{
	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		m_SurfaceFormat.format,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
	);

	vk::ImageView imageView = m_Device.Logical.createImageView(imageViewCreateInfo);

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1ui32
	);

	vk::CommandBuffer commandBuffer = m_Device.Logical.allocateCommandBuffers(commandBufferAllocateInfo)[0];

	return Frame(image, imageView, commandBuffer, frameIndex);
}