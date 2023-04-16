#pragma once

#include "RendererModule.h"
#include "VulkanUtils.h"

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanDeviceHandler;
class VulkanRayTracingPipeline;

class RENDERER_API VulkanShaderBindingTable final
{

public:
	VulkanShaderBindingTable() = default;
	VulkanShaderBindingTable(const VulkanDeviceHandler* vkDevice)
		: m_VkDevice(vkDevice)
	{}
	~VulkanShaderBindingTable() = default;

public:
	void CreateShaderBindingTable();
	void DestroyShaderBindingTable();

private:

public:
	vk::StridedDeviceAddressRegionKHR GetRaygenShaderBindingTable() const { return m_RaygenShaderBindingTable; }
	vk::StridedDeviceAddressRegionKHR GetMissShaderBindingTable() const { return m_MissShaderBindingTable; }
	vk::StridedDeviceAddressRegionKHR GetHitShaderBindingTable() const { return m_HitShaderBindingTable; }
	vk::StridedDeviceAddressRegionKHR GetCallableShaderBindingTable() const { return m_CallableShaderBindingTable; }

	void SetVulkanDevice(const VulkanDeviceHandler* vkDevice) { m_VkDevice = vkDevice; }
	void SetVulkanRayTracingPipeline(const VulkanRayTracingPipeline* vkPipeline) { m_VkPipeline = vkPipeline; }
	void SetDispatchLoaderDynamic(const vk::DispatchLoaderDynamic* dldi) { m_Dldi = dldi; }

private:
	const VulkanDeviceHandler* m_VkDevice = nullptr;
	const VulkanRayTracingPipeline* m_VkPipeline = nullptr;
	const vk::DispatchLoaderDynamic* m_Dldi;

	vk::Buffer m_SbtBuffer;
	vk::DeviceMemory m_SbtBufferMemory;

	vk::StridedDeviceAddressRegionKHR m_RaygenShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_MissShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_HitShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_CallableShaderBindingTable;
};
