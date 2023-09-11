#include "VulkanBuffer.h"
#include "Vulkan/Log.h"
#include "VulkanContext.h"

VulkanBuffer VulkanBuffer::Create(const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties, const void* pNextMemoryAllocateChain)
{
	// Create the buffer
	vk::BufferCreateInfo bufferInfo(
		vk::BufferCreateFlags(),
		size,
		usage,
		vk::SharingMode::eExclusive
	);
	vk::Buffer buffer = VulkanContext::GetDevice().createBuffer(bufferInfo, nullptr, VulkanContext::GetDispatcher());

	// Find the memory type index
	vk::MemoryRequirements memoryRequirements = VulkanContext::GetDevice().getBufferMemoryRequirements(buffer, VulkanContext::GetDispatcher());
	vk::PhysicalDeviceMemoryProperties memoryProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties(VulkanContext::GetDispatcher());
	uint32_t memoryTypeIndex = 0;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			memoryTypeIndex = i;
			break;
		}
	}

	// Allocate the memory
	vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		memoryTypeIndex,
		pNextMemoryAllocateChain
	);
	vk::DeviceMemory bufferMemory = VulkanContext::GetDevice().allocateMemory(
		allocateInfo,
		nullptr,
		VulkanContext::GetDispatcher()
	);

	// Bind the buffer to the memory
	VulkanContext::GetDevice().bindBufferMemory(buffer, bufferMemory, 0);

	// Create the buffer wrapper
	return VulkanBuffer(
		buffer,
		size,
		bufferMemory
	);
}

VulkanBuffer::~VulkanBuffer()
{
	if (static_cast<VkBuffer>(*this))
		VulkanContext::GetDevice().destroyBuffer(*this);

	if (m_BufferMemory)
		VulkanContext::GetDevice().freeMemory(m_BufferMemory);
}

void* VulkanBuffer::Map(const vk::MemoryMapFlags flags) const
{
	void* data = nullptr;
	vk::Result result = VulkanContext::GetDevice().mapMemory(m_BufferMemory, 0, m_BufferSize, flags, &data);
	if (result != vk::Result::eSuccess)
	{
		VULKAN_LOG(Warning, "Unable to map buffer memory: {:s}", vk::to_string(result));
		return (nullptr);
	}
	return (data);
}

void VulkanBuffer::Unmap() const
{
	VulkanContext::GetDevice().unmapMemory(m_BufferMemory);
}

vk::DeviceAddress VulkanBuffer::GetDeviceAddress() const
{
	return (VulkanContext::GetDevice().getBufferAddress(
		vk::BufferDeviceAddressInfo(*this)
	));
}
