#include "VulkanSwapchain.h"
#include "VulkanContext.h"
#include "Vulkan/Log.h"
#include "Vulkan/ErrorHandling.h"
#include "VulkanDevice.h"

#include <vulkan/vulkan.hpp>

VulkanSwapchain::VulkanSwapchain(
	vk::PresentModeKHR presentMode,
	const vk::SurfaceKHR surface,
	VulkanSwapchainFrame::AxisSize width,
	VulkanSwapchainFrame::AxisSize height
)
	: m_Surface(surface),
	m_VkSwapchain(VK_NULL_HANDLE),
	m_CommandPool(VK_NULL_HANDLE),
	m_PresentMode(vk::PresentModeKHR::eImmediate),
	m_Frames(0),
	m_FrameImageFormat(),
	m_CurrentFrameIndex(0),
	m_CurrentSyncFrameIndex(0)
{
	CreateSwapchain(presentMode, width, height);

	/* COMMAND POOL */

	vk::CommandPoolCreateInfo commandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics
	);

	try {
		m_CommandPool = VulkanContext::GetDevice().createCommandPool(commandPoolCreateInfo);
	}
	catch (vk::SystemError& e)
	{
		VULKAN_LOG(Error, "Failed to create command pool: \"{:s}\"", e.what());
		return;
	}

	VULKAN_LOG(Verbose, "Swapchain properties: ");
	VULKAN_LOG(Verbose, "\tFormat: {:s} ({:s})", vk::to_string(m_FrameImageFormat.format), vk::to_string(m_FrameImageFormat.colorSpace));
	VULKAN_LOG(Verbose, "\tPresent mode: {:s}", vk::to_string(m_PresentMode));
	VULKAN_LOG(Verbose, "\tExtent: {:d}x{:d}", width, height);
	VULKAN_LOG(Verbose, "\tImage count: {:d}", m_Frames.size());

	/* CREATE EACH IMAGES */

	auto swapchainImages = VulkanContext::GetDevice().getSwapchainImagesKHR(m_VkSwapchain);
	m_Frames.reserve(swapchainImages.size());

	for (auto& image : swapchainImages)
		m_Frames.emplace_back(VulkanSwapchainFrame(image, width, height, m_FrameImageFormat.format, m_CommandPool));
}

VulkanSwapchain::~VulkanSwapchain()
{
}

void VulkanSwapchain::Destroy()
{
	VULKAN_LOG(Verbose, "Destroying swapchain...");

	VulkanContext::GetDevice().waitIdle();

	for (auto& frame : m_Frames)
	{
		frame.Destroy(m_CommandPool);
	}
	m_Frames.clear();

	if (m_CommandPool)
	{
		VulkanContext::GetDevice().destroyCommandPool(m_CommandPool);
		m_CommandPool = VK_NULL_HANDLE;
	}

	if (m_VkSwapchain)
	{
		VulkanContext::GetDevice().destroySwapchainKHR(m_VkSwapchain);
		m_VkSwapchain = VK_NULL_HANDLE;
	}

	if (m_Surface)
	{
		VulkanContext::GetInstance().destroySurfaceKHR(m_Surface);
		m_Surface = VK_NULL_HANDLE;
	}
}

const VulkanSwapchainFrame& VulkanSwapchain::AcquireNextFrame()
{
	// Wait for the sync fence, that make sure the frame is not in use
	CHECK_VK_RESULT(
		VulkanContext::GetDevice().waitForFences(
			1, &m_Frames[m_CurrentSyncFrameIndex].GetSync().RenderedFinished,
			VK_TRUE, UINT64_MAX, VulkanContext::GetDispatcher()
		),
		"Waiting for sync fence", Error
	);
	CHECK_VK_RESULT( // Close the frame behind
		VulkanContext::GetDevice().resetFences(
			1, &m_Frames[m_CurrentSyncFrameIndex].GetSync().RenderedFinished,
			VulkanContext::GetDispatcher()
		),
		"Resetting sync fence", Error
	);

	CHECK_VK_RESULT(
		VulkanContext::GetDevice().acquireNextImageKHR(
			m_VkSwapchain,
			UINT64_MAX,
			m_Frames[m_CurrentSyncFrameIndex].GetSync().AcquireImage,
			VK_NULL_HANDLE,
			&m_CurrentFrameIndex,
			VulkanContext::GetDispatcher()
		),
		"Acquiring next swapchain image", Fatal
	);

	return (m_Frames[m_CurrentFrameIndex]);
}

void VulkanSwapchain::PresentFrame()
{
	/* RENDER */

	vk::SubmitInfo submitInfo(
		1, &m_Frames[m_CurrentFrameIndex].GetSync().AcquireImage,
		nullptr,
		1, &m_Frames[m_CurrentFrameIndex].GetCommandBuffer(),
		1, &m_Frames[m_CurrentFrameIndex].GetSync().RenderFinished
	);

	try {
		VulkanContext::GetDevice().getQueue(
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics, 0
		).submit(submitInfo, m_Frames[m_CurrentFrameIndex].GetSync().RenderedFinished);
	}
	catch (vk::SystemError& e)
	{
		VULKAN_LOG(Fatal, "Failed to submit swapchain image: \"{:s}\"", e.what());
		return;
	}

	/* PRESENT */

	vk::PresentInfoKHR presentInfo(
		1, &m_Frames[m_CurrentSyncFrameIndex].GetSync().RenderFinished,
		1, &m_VkSwapchain,
		&m_CurrentFrameIndex
	);

	try {
		vk::Result result = VulkanContext::GetDevice().getQueue(
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Present, 0
		).presentKHR(presentInfo);
		if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
		{
			VULKAN_LOG(Warning, "Swapchain was out of date");
			return;
		}
		else if (result != vk::Result::eSuccess)
		{
			VULKAN_LOG(Fatal, "Failed to present swapchain image: \"{:s}\"", vk::to_string(result));
			return;
		}
	}
	catch (vk::SystemError& e)
	{
		VULKAN_LOG(Fatal, "Failed to present swapchain image: \"{:s}\"", e.what());
		return;
	}

	// Increment the current sync frame index to the next frame
	m_CurrentSyncFrameIndex = (m_CurrentSyncFrameIndex + 1) % m_Frames.size();
}

void VulkanSwapchain::Resize(VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height)
{
	// Recreate the swapchain
	RecreateSwapchain(width, height);
}

bool VulkanSwapchain::IsPresentModeSupported(vk::PresentModeKHR presentMode) const
{
	auto presentsModesAvailable = VulkanContext::GetPhysicalDevice().getSurfacePresentModesKHR(m_Surface);

	for (const auto& availablePresentMode : presentsModesAvailable)
	{
		if (availablePresentMode == presentMode)
			return (true);
	}
	return (false);
}

void VulkanSwapchain::CreateSwapchain(vk::PresentModeKHR presentMode, VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height)
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	CHECK_VK_RESULT(
		VulkanContext::GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_Surface, &surfaceCapabilities),
		"Getting surface capabilities", Error
	);

	vk::SwapchainCreateInfoKHR createInfo(
		vk::SwapchainCreateFlagsKHR(),
		m_Surface,
		0, // Set later (image count)
		static_cast<vk::Format>(0), // Set later (surface format)
		static_cast<vk::ColorSpaceKHR>(0), // Set later (surface colorSpace)
		vk::Extent2D(width, height),
		1,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage,
		vk::SharingMode::eExclusive,
		0, nullptr,
		surfaceCapabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		static_cast<vk::PresentModeKHR>(0), // Set later (present mode)
		VK_TRUE
	);

	/* SURFACE FORMAT/COLORSPACE */

	// TODO: change this to something more modular
	vk::Format preferredFormat = vk::Format::eR8G8B8A8Srgb;
	vk::ColorSpaceKHR preferredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

	// check if image and colorspace are supported
	auto surfaceFormats = VulkanContext::GetPhysicalDevice().getSurfaceFormatsKHR(m_Surface);
	bool found = false;
	VULKAN_LOG(VeryVerbose, "Available surface formats: ");
	for (const auto& surfaceFormat : surfaceFormats)
	{
		VULKAN_LOG(VeryVerbose, "\t{:s} ({:s})", vk::to_string(surfaceFormat.format), vk::to_string(surfaceFormat.colorSpace));
		if (surfaceFormat.format == preferredFormat && surfaceFormat.colorSpace == preferredColorSpace)
		{
			found = true;
			VULKAN_LOG(VeryVerbose, "\t\tSurface format has been found");
#ifdef NO_LOGGING
			break;
#endif // NO_LOGGING
		}
	}
	if (found)
	{
		createInfo.imageFormat = preferredFormat;
		createInfo.imageColorSpace = preferredColorSpace;
	}
	else
	{
		VULKAN_LOG(Warning, "Surface format not supported, use first available.");
		createInfo.imageFormat = surfaceFormats[0].format;
		createInfo.imageColorSpace = surfaceFormats[0].colorSpace;
	}
	m_FrameImageFormat.format = createInfo.imageFormat;
	m_FrameImageFormat.colorSpace = createInfo.imageColorSpace;

	/* PRESENT MODE */

	// Check whether the given present mode is supported, if not use the FIFO mode
	if (IsPresentModeSupported(presentMode) == false)
		m_PresentMode = vk::PresentModeKHR::eFifo; // Guarantied to be available by vulkan
	else
		m_PresentMode = presentMode;
	createInfo.presentMode = m_PresentMode;

	/* IMAGE COUNT */

	createInfo.minImageCount = std::min(
		surfaceCapabilities.minImageCount + 1,
		surfaceCapabilities.maxImageCount
	);

	/* CREATE VK SWAPCHAIN */

	try {
		CHECK_VK_RESULT(
			VulkanContext::GetDevice().createSwapchainKHR(&createInfo, nullptr, &m_VkSwapchain, VulkanContext::GetDispatcher()),
			"Creating swapchain", Error
		);
	}
	catch (vk::SystemError& e)
	{
		VULKAN_LOG(Error, "Failed to create swapchain: \"{:s}\"", e.what());
		return;
	}
}

void VulkanSwapchain::RecreateSwapchain(VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height)
{
	VULKAN_LOG(Verbose, "Recreating swapchain {:d}x{:d}", width, height)

	// Wait for the device to be idle
	VulkanContext::GetDevice().waitIdle();

	// Destroy the old swapchain
	VulkanContext::GetDevice().destroySwapchainKHR(m_VkSwapchain, nullptr, VulkanContext::GetDispatcher());

	// Recreate the swapchain
	CreateSwapchain(m_PresentMode, width, height);

	/* UPDATE/CREATE/DESTROY FRAMES */

	auto newImages = VulkanContext::GetDevice().getSwapchainImagesKHR(m_VkSwapchain);
	uint8_t imageIndex = 0;

	// Resize the frames that are still valid
	while (imageIndex < newImages.size() && imageIndex < m_Frames.size())
	{
		m_Frames[imageIndex].Resize(newImages[imageIndex], width, height, m_FrameImageFormat.format);
		imageIndex++;
	}

	// If there are more images than frames, create new frames
	while (imageIndex < newImages.size())
	{
		m_Frames.emplace_back(VulkanSwapchainFrame(newImages[imageIndex], width, height, m_FrameImageFormat.format, m_CommandPool));
		imageIndex++;
	}

	// If there are more frames than images, destroy the frames
	while (imageIndex < m_Frames.size())
	{
		m_Frames[imageIndex].Destroy(m_CommandPool);
		m_Frames.pop_back();
		imageIndex++;
	}

	// Reset indexes
	m_CurrentFrameIndex = m_CurrentSyncFrameIndex = 0;
}
