#pragma once

#include "Renderer_API.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanInstanceHandler.h"
#include "Vulkan/VulkanDeviceHandler.h"

#include <vulkan/vulkan.hpp>

struct alignas(8) MiddlePosition {
	float x;
	float y;
	float z;
};

class RENDERER_API VulkanAccelerationStructure final
{
public:
	VulkanAccelerationStructure() = default;
	VulkanAccelerationStructure(const VulkanDeviceHandler* device)
		: m_VkDevice(device)
	{}
	~VulkanAccelerationStructure() = default;

public:
	void CreateAccelerationStructure(const vk::CommandBuffer& commandBuffer);
	void DestroyAccelerationStructure();

public:
	vk::AccelerationStructureKHR GetBlas() const { return m_Blas; }
	vk::AccelerationStructureKHR GetTlas() const { return m_Tlas; }

	void SetVulkanDevice(const VulkanDeviceHandler* device) { m_VkDevice = device; }
	void SetDispatchLoaderDynamic(const vk::DispatchLoaderDynamic* dldi) { m_Dldi = dldi; }

private:
	const VulkanDeviceHandler* m_VkDevice = nullptr;
	const vk::DispatchLoaderDynamic* m_Dldi = nullptr;

	vk::Buffer m_AabbBuffer;
	vk::DeviceMemory m_AabbBufferMemory;

	vk::Buffer m_BlasBuffer;
	vk::DeviceMemory m_BlasBufferMemory;

	vk::Buffer m_TlasInstanceBuffer;
	vk::DeviceMemory m_TlasInstanceBufferMemory;

	vk::Buffer m_TlasBuffer;
	vk::DeviceMemory m_TlasBufferMemory;

	vk::AccelerationStructureKHR m_Blas;
	vk::AccelerationStructureKHR m_Tlas;
};
