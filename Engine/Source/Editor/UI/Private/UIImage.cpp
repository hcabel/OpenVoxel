#include "UIImage.h"
#include "VulkanContext.h"
#include "ImGuiBackends.h"

#include "CoreGlobals.h"

UIImage::UIImage(vk::ImageCreateInfo imageCreateInfo, vk::ImageViewCreateInfo viewCreateInfo, vk::SamplerCreateInfo samplerCreateInfo)
	: m_ImageCreateInfo(imageCreateInfo),
	m_ViewCreateInfo(viewCreateInfo),
	m_Sampler(VulkanContext::GetDevice().createSampler(samplerCreateInfo)),
	m_Image(VK_NULL_HANDLE),
	m_ImageView(VK_NULL_HANDLE),
	m_ImageMemory(VK_NULL_HANDLE),
	m_Descriptor(VK_NULL_HANDLE)
{
	Resize(imageCreateInfo.extent.width, imageCreateInfo.extent.height);
}

UIImage::~UIImage()
{
	if (m_Image)
	{
		VulkanContext::GetDevice().waitIdle();

		ImGui_ImplVulkan_RemoveTexture(m_Descriptor);
		VulkanContext::GetDevice().freeMemory(m_ImageMemory);
		VulkanContext::GetDevice().destroyImageView(m_ImageView);
		VulkanContext::GetDevice().destroyImage(m_Image);
		VulkanContext::GetDevice().destroySampler(m_Sampler);
	}
}

void UIImage::Resize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
		return;

	// Destroy old image, if any
	if (m_Image)
	{
		VulkanContext::GetDevice().waitIdle();

		VulkanContext::GetDevice().freeMemory(m_ImageMemory);
		VulkanContext::GetDevice().destroyImageView(m_ImageView);
		VulkanContext::GetDevice().destroyImage(m_Image);
		ImGui_ImplVulkan_RemoveTexture(m_Descriptor);
	}

	// Create new image
	m_ImageCreateInfo.extent = vk::Extent3D(width, height, 1);
	m_Image = VulkanContext::GetDevice().createImage(m_ImageCreateInfo);

	// Allocate/bind memory for image
	vk::MemoryRequirements memoryRequirements = VulkanContext::GetDevice().getImageMemoryRequirements(m_Image);
	vk::MemoryAllocateInfo memoryAllocateInfo(
		memoryRequirements.size,
		VulkanContext::GetDevice().FindMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	);
	m_ImageMemory = VulkanContext::GetDevice().allocateMemory(memoryAllocateInfo, nullptr, VulkanContext::GetDispatcher());

	VulkanContext::GetDevice().bindImageMemory(m_Image, m_ImageMemory, 0);

	// Create viewport image view
	m_ViewCreateInfo.image = m_Image;
	m_ImageView = VulkanContext::GetDevice().createImageView(m_ViewCreateInfo);

	m_Descriptor = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_GENERAL);
}
