#pragma once

#include "RendererModule.h"
#include "Vulkan/VulkanUtils.h"
#include "Vulkan/VulkanInstanceHandler.h"
#include "Vulkan/VulkanDeviceHandler.h"

struct alignas(8) MiddlePosition {
	float x;
	float y;
	float z;
};

class RENDERER_API VulkanAccelerationStructure final
{
public:
	struct MemoryRequirementExtended : public vk::MemoryRequirements
	{
		uint32_t MemoryTypeIndex = 0;
	};


public:
	VulkanAccelerationStructure() = default;
	VulkanAccelerationStructure(const VulkanDeviceHandler* device)
		: m_VkDevice(device)
	{}
	~VulkanAccelerationStructure() = default;

public:
	void CreateAccelerationStructure(const vk::CommandBuffer& commandBuffer);
	void DestroyAccelerationStructure();

private:
	MemoryRequirementExtended FindMemoryRequirement(const vk::Buffer& buffer, vk::MemoryPropertyFlags memoryProperty) const;

public:
	void SetVulkanDevice(const VulkanDeviceHandler* device) { m_VkDevice = device; }
	void CreateDispatchLoaderDynamic(const VulkanInstanceHandler* vkInstance, const VulkanDeviceHandler* vkDevice)
	{
		m_Dldi = new vk::DispatchLoaderDynamic(vkInstance->RawC(), vkGetInstanceProcAddr, vkDevice->Raw(), vkGetDeviceProcAddr);
	}

private:
	const VulkanDeviceHandler* m_VkDevice = nullptr;
	const vk::DispatchLoaderDynamic* m_Dldi = nullptr;

	vk::Buffer BLAS_VoxelBuffer;
	vk::DeviceMemory BLAS_VoxelBufferMemory;
	vk::Buffer BLAS_Buffer;
	vk::DeviceMemory BLAS_BufferMemory;

	vk::Buffer BLAS_ScratchBuffer;
	vk::DeviceMemory BLAS_ScratchMemory;
	vk::Buffer TLAS_InstanceBuffer;
	vk::DeviceMemory TLAS_InstanceMemory;
	vk::Buffer TLAS_Buffer;
	vk::DeviceMemory TLAS_BufferMemory;
	vk::Buffer TLAS_ScratchBuffer;
	vk::DeviceMemory TLAS_ScratchMemory;

	vk::AccelerationStructureKHR BLAS;
	vk::AccelerationStructureKHR TLAS;
};
