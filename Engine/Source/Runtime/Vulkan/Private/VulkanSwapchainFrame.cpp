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
}

void VulkanSwapchainFrame::Begin() const
{
	m_CommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	m_CommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
}

void VulkanSwapchainFrame::End() const
{
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

	if (m_ImageView)
		VulkanContext::GetDevice().destroyImageView(m_ImageView, nullptr, VulkanContext::GetDispatcher());

	if (m_CommandBuffer)
		VulkanContext::GetDevice().freeCommandBuffers(commandPool, 1, &m_CommandBuffer, VulkanContext::GetDispatcher());
}

void VulkanSwapchainFrame::Resize(vk::Image& newImage, VulkanSwapchainFrame::AxisSize width, VulkanSwapchainFrame::AxisSize height, vk::Format imageFormat)
{
	m_Width = width;
	m_Height = height;
	m_Image = newImage;

	// Destroy old stuff
	VulkanContext::GetDevice().destroyImageView(m_ImageView, nullptr, VulkanContext::GetDispatcher());

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
}
