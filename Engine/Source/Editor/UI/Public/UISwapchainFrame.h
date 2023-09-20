#pragma once

#include "UI_API.h"
#include "VulkanSwapchainFrame.h"

#include <vulkan/vulkan.hpp>

class UI_API UISwapchainFrame : public VulkanSwapchainFrame
{
public:
	UISwapchainFrame()
		: VulkanSwapchainFrame(),
		m_FrameBuffer(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE)
	{}
	UISwapchainFrame(
		vk::Image& image,
		AxisSize width,
		AxisSize height,
		vk::Format imageFormat,
		vk::CommandPool& swapchainCmdPool,
		vk::RenderPass imguiRenderPass
	);

public:
	virtual const vk::CommandBuffer& Begin() const override;
	virtual void End() const override;

protected:
	virtual void Destroy(const vk::CommandPool commandPool) override;
	virtual void Resize(vk::Image& image, AxisSize width, AxisSize height, vk::Format imageFormat) override;

protected:
	vk::Framebuffer m_FrameBuffer;
	vk::RenderPass m_RenderPass;

	friend class UISwapchain;
};
