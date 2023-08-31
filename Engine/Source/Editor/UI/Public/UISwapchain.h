#pragma once

#include "UISwapchainFrame.h"

#include <Vulkan/vulkan.hpp>
#include <vector>

class UISwapchain
{
public:
	UISwapchain()
		: m_Surface(VK_NULL_HANDLE),
		m_VkSwapchain(VK_NULL_HANDLE),
		m_CommandPool(VK_NULL_HANDLE),
		m_PresentMode(vk::PresentModeKHR::eFifo),
		m_Frames(0),
		m_CurrentFrameIndex(0),
		m_ImguiRenderPass(VK_NULL_HANDLE)
	{}
	UISwapchain(
		vk::PresentModeKHR presentMode,
		const vk::SurfaceKHR surface,
		VulkanSwapchainFrame::AxisSize width,
		VulkanSwapchainFrame::AxisSize height,
		vk::RenderPass imGuiRenderPass
	);
	~UISwapchain();

public:
	void Destroy();

	const UISwapchainFrame& AcquireNextFrame();
	void PresentFrame();

	void Resize(UISwapchainFrame::AxisSize width, UISwapchainFrame::AxisSize height);

	operator bool() const { return m_VkSwapchain; }

	static vk::SurfaceFormatKHR SelectSurfaceFormat(vk::SurfaceKHR surface);

public:
	bool IsPresentModeSupported(vk::PresentModeKHR presentMode) const;
	__forceinline UISwapchainFrame::AxisSize GetFrameWidth() const { return (m_Frames.empty() ? 0 : m_Frames[0].GetWidth()); }
	__forceinline UISwapchainFrame::AxisSize GetFrameHeight() const { return (m_Frames.empty() ? 0 : m_Frames[0].GetHeight()); }
	__forceinline vk::Extent2D GetFrameExtent() const { return vk::Extent2D(GetFrameWidth(), GetFrameHeight()); }
	__forceinline uint32_t GetFrameCount() const { return static_cast<uint32_t>(m_Frames.size()); }
	__forceinline uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
	__forceinline const UISwapchainFrame& GetCurrentFrame() const { return m_Frames[m_CurrentFrameIndex]; }
	__forceinline const std::vector<UISwapchainFrame>& GetAllFrames() const { return m_Frames; }
	__forceinline vk::Format GetFrameFormat() const { return m_FrameImageFormat.format; }
	__forceinline vk::ColorSpaceKHR GetFrameColorSpace() const { return m_FrameImageFormat.colorSpace; }

protected:
	void CreateSwapchain(vk::PresentModeKHR presentMode, UISwapchainFrame::AxisSize width, UISwapchainFrame::AxisSize height);
	void RecreateSwapchain(UISwapchainFrame::AxisSize width, UISwapchainFrame::AxisSize height);

protected:
	vk::SurfaceKHR m_Surface;
	vk::SwapchainKHR m_VkSwapchain;
	vk::CommandPool m_CommandPool;
	vk::PresentModeKHR m_PresentMode;
	vk::RenderPass m_ImguiRenderPass;

	/* This is the array that contain of frames, and their data */
	std::vector<UISwapchainFrame> m_Frames;
	vk::SurfaceFormatKHR m_FrameImageFormat;
	uint32_t m_CurrentFrameIndex;
};
