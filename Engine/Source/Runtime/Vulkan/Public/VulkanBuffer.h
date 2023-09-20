#pragma once

#include "Vulkan_API.h"

#include <vulkan/vulkan.hpp>

class VULKAN_API VulkanBuffer : public vk::Buffer
{
// Start region for the constructors and destructors
#pragma region Lifecycle
public:
	/**
	 * @brief Create a vk::Buffer and wrap it in a VulkanBuffer
	 *
	 * @param size The size of the buffer
	 * @param usage The usage of the buffer
	 * @param properties The memory properties of the buffer
	 * @param pNextMemoryAllocateChain, (optional) The pNext chain for the memory allocation
	 * @return The new VulkanBuffer wrapper instance (not allocated on the heap)
	 */
	static VulkanBuffer Create(
		const vk::DeviceSize size,
		const vk::BufferUsageFlags usage,
		const vk::MemoryPropertyFlags properties,
		const void* pNextMemoryAllocateChain = nullptr
	);
	/**
	 * @brief default constructor, will not create the buffer, see @ref Create for that
	 */
	VulkanBuffer() = default;
	/**
	 * @brief null pointer constructor, same has using the default constructor
	 */
	VulkanBuffer(std::nullptr_t) : VulkanBuffer() {}
	~VulkanBuffer();
	/**
	 * @brief Move constructor, will create a new instance with the data from the other instance, and un-validate the other instance
	 *
	 * @param rhs The VulkanBuffer to move from
	 */
	VulkanBuffer(VulkanBuffer&& other) noexcept;
	/**
	 * @brief Move assignment operator, will move the data from the rhs to the lhs, and un-validate the rhs
	 * @param rhs The VulkanBuffer to move
	 * @return The new VulkanBuffer
	 */
	VulkanBuffer& operator=(VulkanBuffer&& rhs) noexcept;

protected:
	/**
	 * @note Should only be called by @ref Create
	 * @brief Wrapper constructor, will take all related data and wrap it in a VulkanBuffer
	 */
	VulkanBuffer(vk::Buffer buffer, vk::DeviceSize bufferSize, vk::DeviceMemory bufferMemory) noexcept
		: vk::Buffer(buffer),
		m_BufferSize(bufferSize),
		m_BufferMemory(bufferMemory)
	{}
#pragma endregion Lifecycle

public:
	/**
	 * @brief Check if the VulkanBuffer is valid
	 */
	__forceinline operator bool() const { return (vk::Buffer::operator bool()); }

public:
	void* Map(const vk::MemoryMapFlags flags = vk::MemoryMapFlags()) const;
	void Unmap() const;

public:
	__forceinline bool IsValid() const { return (this->operator bool()); }
	vk::DeviceAddress GetDeviceAddress() const;

protected:
	vk::DeviceSize m_BufferSize;
	vk::DeviceMemory m_BufferMemory;
};
