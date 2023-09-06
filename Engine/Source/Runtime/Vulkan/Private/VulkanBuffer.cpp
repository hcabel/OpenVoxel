#include "VulkanBuffer.h"
#include "VulkanContext.h"

VulkanBuffer::VulkanBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties)
{
	vk::BufferCreateInfo bufferInfo(
		vk::BufferCreateFlags(),
		size,
		usage,
		vk::SharingMode::eExclusive
	);
	*this = VulkanBuffer(VulkanContext::GetDevice().createBuffer(bufferInfo), std::move(*this));

	vk::MemoryRequirements memoryRequirements = VulkanContext::GetDevice().getBufferMemoryRequirements(*this);

	vk::PhysicalDeviceMemoryProperties memoryProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();
	uint32_t memoryTypeIndex = 0;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			memoryTypeIndex = i;
			break;
		}
	}

	vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		memoryTypeIndex
	);
	m_BufferMemory = VulkanContext::GetDevice().allocateMemory(allocateInfo);

	VulkanContext::GetDevice().bindBufferMemory(*this, m_BufferMemory, 0);
}

VulkanBuffer::~VulkanBuffer()
{
	if (static_cast<vk::Buffer>(*this))
		VulkanContext::GetDevice().destroyBuffer(*this);
	if (m_BufferMemory)
		VulkanContext::GetDevice().freeMemory(m_BufferMemory);

}

void* VulkanBuffer::Map(const vk::MemoryMapFlags flags) const
{
	void* data;
	VulkanContext::GetDevice().mapMemory(m_BufferMemory, 0, m_BufferSize, flags, &data);
	return (data);
}

void VulkanBuffer::Unmap() const
{
	VulkanContext::GetDevice().unmapMemory(m_BufferMemory);
}
