#pragma once

#include "Vulkan_API.h"

#include <vulkan/vulkan.hpp>

class VULKAN_API VulkanBuffer : public vk::Buffer
{

protected:
	// m_buffer is a private member of vk::Buffer, so to assigned it I recreate the object and then I move it to the current object
	VulkanBuffer(vk::Buffer buffer, VulkanBuffer&& self)
		: vk::Buffer(buffer),
		m_BufferSize(std::move(self.m_BufferSize)),
		m_BufferMemory(std::move(self.m_BufferMemory))
	{
		self.m_BufferSize = 0;
		self.m_BufferMemory = nullptr;
	}

public:
	VulkanBuffer()
		: vk::Buffer(nullptr),
		m_BufferSize(0),
		m_BufferMemory(nullptr)
	{}
	VulkanBuffer(std::nullptr_t) : VulkanBuffer() {} // default constructor 2 (for null assignment)
	VulkanBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties);
	~VulkanBuffer();

	VulkanBuffer& operator=(VulkanBuffer&& self) noexcept
	{
		static_cast<vk::Buffer>(*this).operator=(self);
		m_BufferSize = std::move(self.m_BufferSize);
		m_BufferMemory = std::move(self.m_BufferMemory);
		self.m_BufferSize = 0;
		self.m_BufferMemory = nullptr;
		return *this;
	}

public:
	void* Map(const vk::MemoryMapFlags flags = vk::MemoryMapFlags()) const;
	void Unmap() const;

	vk::DeviceAddress GetDeviceAddress() const;

protected:
	vk::DeviceSize m_BufferSize;
	vk::DeviceMemory m_BufferMemory;
};
