#include "VulkanSwapchainFrame.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

VulkanSwapchainFrame::VulkanSwapchainFrame(
	vk::Image& image,
	VulkanSwapchainFrame::AxisSize width,
	VulkanSwapchainFrame::AxisSize height,
	vk::Format imageFormat,
	vk::CommandPool& swapchainCmdPool
)
	: m_Image(image), m_Width(width), m_Height(height)
{
	/* IMAGE VIEW */

	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		imageFormat,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
	);
	m_ImageView = VulkanContext::GetDevice().createImageView(imageViewCreateInfo);

	/* COMMAND BUFFER */

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo(swapchainCmdPool, vk::CommandBufferLevel::ePrimary, 1);
	m_CommandBuffer = VulkanContext::GetDevice().allocateCommandBuffers(commandBufferAllocateInfo)[0];

	/* SYNC */

	// Rendered finished fence
	m_Sync.RenderedFinished = VulkanContext::GetDevice().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	m_Sync.AcquireImage = VulkanContext::GetDevice().createSemaphore(vk::SemaphoreCreateInfo());
	m_Sync.RenderFinished = VulkanContext::GetDevice().createSemaphore(vk::SemaphoreCreateInfo());

	/* DUMMY RENDER PASS */

	std::array<vk::AttachmentDescription, 1> attachments = {};
	attachments[0].format = imageFormat;
	attachments[0].samples = vk::SampleCountFlagBits::e1;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear,
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;

	vk::SubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
	subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo renderPassCreateInfo(
		vk::RenderPassCreateFlags(),
		static_cast<uint32_t>(attachments.size()), attachments.data(),
		1, &subpass,
		1, &subpassDependency
	);
	VulkanContext::GetDevice().createRenderPass(&renderPassCreateInfo, nullptr, &m_RP);

	/* FRAMEBUFFER */

	vk::FramebufferCreateInfo framebufferCreateInfo(
		vk::FramebufferCreateFlags(),
		m_RP,
		1, &m_ImageView,
		m_Width, m_Height, 1
	);
	m_FB = VulkanContext::GetDevice().createFramebuffer(framebufferCreateInfo);
}

void VulkanSwapchainFrame::Begin() const
{
	m_CommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	m_CommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
}

void VulkanSwapchainFrame::End() const
{
	// Dummy render pass to set a background color, allow me to debug the swapchain resizing
	vk::ClearValue clearValue(vk::ClearColorValue(std::array<float, 4>{1.0f, 0.5f, 0.5f, 1.0f}));
	vk::RenderPassBeginInfo renderPassBeginInfo(
		m_RP,
		m_FB,
		vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_Width, m_Height)),
		1, &clearValue
	);
	m_CommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	vk::ClearAttachment clearAttachment(
		vk::ImageAspectFlagBits::eColor,
		0,
		clearValue
	);
	vk::ClearRect clearRect(
		vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_Width, m_Height)),
		0, 1
	);
	m_CommandBuffer.clearAttachments(clearAttachment, clearRect);
	m_CommandBuffer.endRenderPass();

	m_CommandBuffer.end();
}

void VulkanSwapchainFrame::Destroy(const vk::CommandPool commandPool)
{
	VulkanContext::GetDevice().waitIdle();

	if (m_Sync.RenderedFinished)
	{
		VulkanContext::GetDevice().waitForFences(m_Sync.RenderedFinished, true, UINT64_MAX, VulkanContext::GetDispatcher());
		VulkanContext::GetDevice().destroyFence(m_Sync.RenderedFinished, nullptr, VulkanContext::GetDispatcher());
	}

	if (m_Sync.AcquireImage)
		VulkanContext::GetDevice().destroySemaphore(m_Sync.AcquireImage, nullptr, VulkanContext::GetDispatcher());

	if (m_Sync.RenderFinished)
		VulkanContext::GetDevice().destroySemaphore(m_Sync.RenderFinished, nullptr, VulkanContext::GetDispatcher());

	if (m_FB)
		VulkanContext::GetDevice().destroyFramebuffer(m_FB, nullptr, VulkanContext::GetDispatcher());

	if (m_RP)
		VulkanContext::GetDevice().destroyRenderPass(m_RP, nullptr, VulkanContext::GetDispatcher());

	if (m_ImageView)
		VulkanContext::GetDevice().destroyImageView(m_ImageView, nullptr, VulkanContext::GetDispatcher());

	if (m_CommandBuffer)
		VulkanContext::GetDevice().freeCommandBuffers(commandPool, 1, &m_CommandBuffer, VulkanContext::GetDispatcher());
}

void VulkanSwapchainFrame::Resize(vk::Image& newImage, VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height, vk::Format imageFormat)
{
	m_Width = width;
	m_Height = height;

	// Destroy old stuff
	VulkanContext::GetDevice().destroyImageView(m_ImageView, nullptr, VulkanContext::GetDispatcher());
	VulkanContext::GetDevice().destroyFramebuffer(m_FB, nullptr, VulkanContext::GetDispatcher());

	/* IMAGE VIEW */

	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		newImage,
		vk::ImageViewType::e2D,
		imageFormat,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
	);
	m_ImageView = VulkanContext::GetDevice().createImageView(imageViewCreateInfo);

	/* FRAME BUFFER */

	vk::FramebufferCreateInfo framebufferCreateInfo(
		vk::FramebufferCreateFlags(),
		m_RP,
		1, &m_ImageView,
		m_Width, m_Height, 1
	);
	m_FB = VulkanContext::GetDevice().createFramebuffer(framebufferCreateInfo);
}
