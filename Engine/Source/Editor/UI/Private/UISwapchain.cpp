
#include "UISwapchain.h"
#include "VulkanContext.h"
#include "UIGlobals.h"
#include "ImGuiBackends.h"

#include <vulkan/vulkan.hpp>

UISwapchain::UISwapchain(
	vk::PresentModeKHR presentMode,
	const vk::SurfaceKHR surface,
	UISwapchainFrame::AxisSize width,
	UISwapchainFrame::AxisSize height,
	vk::RenderPass imGuiRenderPass
)
	: m_Surface(surface),
	m_VkSwapchain(VK_NULL_HANDLE),
	m_CommandPool(VK_NULL_HANDLE),
	m_PresentMode(vk::PresentModeKHR::eImmediate),
	m_Frames(0),
	m_FrameImageFormat(),
	m_CurrentFrameIndex(0),
	m_ImguiRenderPass(imGuiRenderPass)
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
		UI_LOG(Error, "Failed to create command pool: \"{:s}\"", e.what());
		return;
	}

	/* CREATE EACH IMAGES */

	auto swapchainImages = VulkanContext::GetDevice().getSwapchainImagesKHR(m_VkSwapchain);
	m_Frames.reserve(swapchainImages.size());

	for (auto& image : swapchainImages)
	{
		m_Frames.emplace_back(
			UISwapchainFrame(image, width, height, m_FrameImageFormat.format, m_CommandPool, imGuiRenderPass)
		);
	}

	UI_LOG(Verbose, "Swapchain properties: ");
	UI_LOG(Verbose, "\tFormat: {:s} ({:s})", vk::to_string(m_FrameImageFormat.format), vk::to_string(m_FrameImageFormat.colorSpace));
	UI_LOG(Verbose, "\tPresent mode: {:s}", vk::to_string(m_PresentMode));
	UI_LOG(Verbose, "\tExtent: {:d}x{:d}", width, height);
	UI_LOG(Verbose, "\tImage count: {:d}", m_Frames.size());
}

UISwapchain::~UISwapchain()
{
}

void UISwapchain::Destroy()
{
	UI_LOG(Verbose, "Destroying swapchain...");

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

const UISwapchainFrame& UISwapchain::AcquireNextFrame()
{
	// Wait for the sync fence, that make sure the frame is not in use
	VulkanContext::GetDevice().waitForFences(
		1, &m_Frames[m_CurrentFrameIndex].GetSync().RenderedFinished,
		VK_TRUE, UINT64_MAX, VulkanContext::GetDispatcher()
	);
	VulkanContext::GetDevice().resetFences(
		1, &m_Frames[m_CurrentFrameIndex].GetSync().RenderedFinished,
		VulkanContext::GetDispatcher()
	);

	VulkanContext::GetDevice().acquireNextImageKHR(
		m_VkSwapchain,
		UINT64_MAX,
		m_Frames[m_CurrentFrameIndex].GetSync().AcquireImage,
		VK_NULL_HANDLE,
		&m_CurrentFrameIndex,
		VulkanContext::GetDispatcher()
	);

	return (m_Frames[m_CurrentFrameIndex]);
}

void UISwapchain::PresentFrame()
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
		UI_LOG(Fatal, "Failed to submit swapchain image: \"{:s}\"", e.what());
		return;
	}

	/* PRESENT */

	vk::PresentInfoKHR presentInfo(
		1, &m_Frames[m_CurrentFrameIndex].GetSync().RenderFinished,
		1, &m_VkSwapchain,
		&m_CurrentFrameIndex
	);

	try {
		vk::Result result = VulkanContext::GetDevice().getQueue(
			VulkanContext::GetDevice().GetQueueFamilyIndicies().Present, 0
		).presentKHR(presentInfo);
		if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
		{
			UI_LOG(Warning, "Swapchain was out of date");
			return;
		}
		else if (result != vk::Result::eSuccess)
		{
			UI_LOG(Fatal, "Failed to present swapchain image: \"{:s}\"", vk::to_string(result));
			return;
		}
	}
	catch (vk::SystemError& e)
	{
		UI_LOG(Fatal, "Failed to present swapchain image: \"{:s}\"", e.what());
		return;
	}

	// Increment the current sync frame index to the next frame
	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_Frames.size();
}

void UISwapchain::Resize(UISwapchainFrame::AxisSize width, UISwapchainFrame::AxisSize height)
{
	// Recreate the swapchain
	RecreateSwapchain(width, height);
}

vk::SurfaceFormatKHR UISwapchain::SelectSurfaceFormat(vk::SurfaceKHR surface)
{
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	return (ImGui_ImplVulkanH_SelectSurfaceFormat(VulkanContext::GetPhysicalDevice(), surface, requestSurfaceImageFormat, 4, requestSurfaceColorSpace));
}

bool UISwapchain::IsPresentModeSupported(vk::PresentModeKHR presentMode) const
{
	auto presentsModesAvailable = VulkanContext::GetPhysicalDevice().getSurfacePresentModesKHR(m_Surface);

	for (const auto& availablePresentMode : presentsModesAvailable)
	{
		if (availablePresentMode == presentMode)
			return (true);
	}
	return (false);
}

void UISwapchain::CreateSwapchain(vk::PresentModeKHR presentMode, UISwapchainFrame::AxisSize width, UISwapchainFrame::AxisSize height)
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	VulkanContext::GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_Surface, &surfaceCapabilities);

	vk::SwapchainCreateInfoKHR createInfo(
		vk::SwapchainCreateFlagsKHR(),
		m_Surface,
		0, // Set later (image count)
		static_cast<vk::Format>(0), // Set later (surface format)
		static_cast<vk::ColorSpaceKHR>(0), // Set later (surface colorSpace)
		vk::Extent2D(width, height),
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0, nullptr,
		surfaceCapabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		static_cast<vk::PresentModeKHR>(0), // Set later (present mode)
		VK_TRUE
	);

	/* SURFACE FORMAT/COLORSPACE */
	m_FrameImageFormat = SelectSurfaceFormat(m_Surface);
	createInfo.imageFormat = m_FrameImageFormat.format;
	createInfo.imageColorSpace = m_FrameImageFormat.colorSpace;

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
		VulkanContext::GetDevice().createSwapchainKHR(&createInfo, nullptr, &m_VkSwapchain, VulkanContext::GetDispatcher());
	}
	catch (vk::SystemError& e)
	{
		UI_LOG(Error, "Failed to create swapchain: \"{:s}\"", e.what());
		return;
	}
}

void UISwapchain::RecreateSwapchain(UISwapchainFrame::AxisSize width, UISwapchainFrame::AxisSize height)
{
	UI_LOG(Verbose, "Recreating swapchain {:d}x{:d}", width, height)

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
		m_Frames.emplace_back(
			UISwapchainFrame(newImages[imageIndex], width, height, m_FrameImageFormat.format, m_CommandPool, m_ImguiRenderPass)
		);
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
	m_CurrentFrameIndex = 0;
}
