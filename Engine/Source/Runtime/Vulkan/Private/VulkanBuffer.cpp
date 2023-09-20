#include "VulkanBuffer.h"
#include "Vulkan/Log.h"
#include "VulkanContext.h"

#pragma region Lifecycle
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

	// Allocate the memory
	vk::MemoryRequirements memoryRequirements = VulkanContext::GetDevice().getBufferMemoryRequirements(buffer, VulkanContext::GetDispatcher());
	vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		VulkanContext::GetDevice().FindMemoryType(memoryRequirements.memoryTypeBits, properties),
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
	if (IsValid())
	{
		VulkanContext::GetDevice().destroyBuffer(*this);
		VulkanContext::GetDevice().freeMemory(m_BufferMemory);
	}
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	: vk::Buffer(std::move(other)),
	m_BufferSize(std::move(other.m_BufferSize)),
	m_BufferMemory(std::move(other.m_BufferMemory))
{
	other.vk::Buffer::operator=(VK_NULL_HANDLE);
	other.m_BufferSize = 0;
	other.m_BufferMemory = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& rhs) noexcept
{
	if (this != &rhs) // If not the same object
	{
		vk::Buffer::operator=(std::move(rhs));
		m_BufferSize = std::move(rhs.m_BufferSize);
		m_BufferMemory = std::move(rhs.m_BufferMemory);

		rhs.vk::Buffer::operator=(VK_NULL_HANDLE);
		rhs.m_BufferSize = 0;
		rhs.m_BufferMemory = nullptr;
	}
	return *this;
}
#pragma endregion Lifecycle

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
