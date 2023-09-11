#pragma once

#include "Vulkan_API.h"

#include <vulkan/vulkan.hpp>

class VULKAN_API VulkanBuffer : public vk::Buffer
{

protected:
	VulkanBuffer(vk::Buffer buffer, vk::DeviceSize bufferSize, vk::DeviceMemory bufferMemory)
		: vk::Buffer(buffer),
		m_BufferSize(bufferSize),
		m_BufferMemory(bufferMemory)
	{}

public:
	static VulkanBuffer Create(
		const vk::DeviceSize size,
		const vk::BufferUsageFlags usage,
		const vk::MemoryPropertyFlags properties,
		const void* pNextMemoryAllocateChain = nullptr
	);
	VulkanBuffer& operator=(VulkanBuffer&& rhs) noexcept
	{
		static_cast<vk::Buffer*>(this)->operator=(rhs);
		static_cast<vk::Buffer&&>(rhs).operator=(VK_NULL_HANDLE);

		m_BufferSize = std::move(rhs.m_BufferSize);
		rhs.m_BufferSize = 0;

		m_BufferMemory = std::move(rhs.m_BufferMemory);
		rhs.m_BufferMemory = nullptr;

		return *this;
	}
	VulkanBuffer()
		: vk::Buffer(nullptr),
		m_BufferSize(0),
		m_BufferMemory(nullptr)
	{}
	VulkanBuffer(std::nullptr_t) : VulkanBuffer() {} // default constructor 2 (for null assignment)
	~VulkanBuffer();

public:
	void* Map(const vk::MemoryMapFlags flags = vk::MemoryMapFlags()) const;
	void Unmap() const;

	vk::DeviceAddress GetDeviceAddress() const;

protected:
	vk::DeviceSize m_BufferSize;
	vk::DeviceMemory m_BufferMemory;
};
