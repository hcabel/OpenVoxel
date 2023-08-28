#pragma once

#include "Vulkan_API.h"
#include "VulkanSwapchainFrame.h"

#include <Vulkan/vulkan.hpp>
#include <vector>

namespace vk
{
	enum class PresentModeKHR;
}

/**
 * This is a swapchain, it is a collection of images that you can draw to one at a time.
 * The thing is that you cant draw to an image that is currently being displayed on the screen.
 * To fix this issue we create a swapchain that will be use to get the next image that we can draw to.
 * once we finish our drawing we present the frame and ask for the next image, the next image being the one that was just displayed.
 * the technique I just describe is called double buffering, but we can also use triple buffering,
 * and other technique to make sure that we present the right frame to the screen at the right time.
 *
 * @warning I'm currently only supporting vulkan so I will only implement the vulkan version of the swapchain.
 *     In the far future I'll create a vulkan specific swapchain class and this class will become a base class for all swapchain.
 */
class VULKAN_API VulkanSwapchain
{
public:
	VulkanSwapchain()
		: m_Surface(VK_NULL_HANDLE),
		m_VkSwapchain(VK_NULL_HANDLE),
		m_CommandPool(VK_NULL_HANDLE),
		m_PresentMode(vk::PresentModeKHR::eFifo),
		m_Frames(0),
		m_CurrentFrameIndex(0),
		m_CurrentSyncFrameIndex(0)
	{}
	VulkanSwapchain(vk::PresentModeKHR presentMode, const vk::SurfaceKHR surface, VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height);
	~VulkanSwapchain();

public:
	void Destroy();

	const VulkanSwapchainFrame& AcquireNextFrame();
	void PresentFrame();

	void Resize(VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height);

public:
	bool IsPresentModeSupported(vk::PresentModeKHR presentMode) const;
	__forceinline VulkanSwapchainFrame::AxisSize GetFrameWidth() const { return (m_Frames.empty() ? 0 : m_Frames[0].GetWidth()); }
	__forceinline VulkanSwapchainFrame::AxisSize GetFrameHeight() const { return (m_Frames.empty() ? 0 : m_Frames[0].GetHeight()); }

protected:
	void CreateSwapchain(vk::PresentModeKHR presentMode, VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height);
	void RecreateSwapchain(VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height);

protected:
	vk::SurfaceKHR m_Surface;
	vk::SwapchainKHR m_VkSwapchain;
	vk::CommandPool m_CommandPool;
	vk::PresentModeKHR m_PresentMode;

	/* This is the array that contain of frames, and their data */
	std::vector<VulkanSwapchainFrame> m_Frames;
	vk::SurfaceFormatKHR m_FrameImageFormat;
	uint32_t m_CurrentFrameIndex;
	uint8_t m_CurrentSyncFrameIndex;
};
