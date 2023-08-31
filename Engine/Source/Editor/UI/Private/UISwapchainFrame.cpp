#include "UISwapchainFrame.h"
#include "VulkanContext.h"
#include "ImGuiBackends.h"
#include "UIGlobals.h"

#include <imgui.h>

UISwapchainFrame::UISwapchainFrame(
	vk::Image &image,
	AxisSize width,
	AxisSize height,
	vk::Format imageFormat,
	vk::CommandPool &swapchainCmdPool,
	vk::RenderPass imguiRenderPass
)
	: VulkanSwapchainFrame(image, width, height, imageFormat, swapchainCmdPool),
	m_FrameBuffer(VK_NULL_HANDLE),
	m_RenderPass(imguiRenderPass)
{
	/* FRAME BUFFER */

	vk::FramebufferCreateInfo framebufferCreateInfo(
		vk::FramebufferCreateFlags(),
		m_RenderPass,
		1, &m_ImageView,
		m_Width,
		m_Height,
		1
	);
	m_FrameBuffer = VulkanContext::GetDevice().createFramebuffer(framebufferCreateInfo);
}

void UISwapchainFrame::Begin() const
{
	VulkanSwapchainFrame::Begin();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UISwapchainFrame::End() const
{
	ImGui::Render();

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	vk::ClearValue clearColor(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_CommandBuffer.beginRenderPass(
		vk::RenderPassBeginInfo(
			m_RenderPass,
			m_FrameBuffer,
			vk::Rect2D(vk::Offset2D(0, 0), GetExtent()),
			1, &clearColor
		),
		vk::SubpassContents::eInline
	);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_CommandBuffer);
	m_CommandBuffer.endRenderPass();

	VulkanSwapchainFrame::End();
}

void UISwapchainFrame::Destroy(const vk::CommandPool commandPool)
{
	VulkanSwapchainFrame::Destroy(commandPool);

	if (m_FrameBuffer)
		VulkanContext::GetDevice().destroyFramebuffer(m_FrameBuffer, nullptr, VulkanContext::GetDispatcher());
}

void UISwapchainFrame::Resize(vk::Image &image, AxisSize width, AxisSize height, vk::Format imageFormat)
{
	VulkanSwapchainFrame::Resize(image, width, height, imageFormat);

	/* FRAME BUFFER */
	VulkanContext::GetDevice().destroyFramebuffer(m_FrameBuffer, nullptr, VulkanContext::GetDispatcher());

	vk::FramebufferCreateInfo framebufferCreateInfo(
		vk::FramebufferCreateFlags(),
		m_RenderPass,
		1, &m_ImageView,
		m_Width,
		m_Height,
		1
	);
	m_FrameBuffer = VulkanContext::GetDevice().createFramebuffer(framebufferCreateInfo);
}
