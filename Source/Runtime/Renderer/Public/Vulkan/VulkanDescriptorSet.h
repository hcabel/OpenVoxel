#pragma once

#include "RendererModule.h"
#include "Vulkan/VulkanUtils.h"

#include <vulkan/vulkan.hpp>

class VulkanDeviceHandler;
class VulkanAccelerationStructure;

class RENDERER_API VulkanDescriptorSet final
{

public:
	VulkanDescriptorSet() = default;
	VulkanDescriptorSet(const VulkanDeviceHandler* vkDevice)
		: m_VkDevice(vkDevice)
	{}
	~VulkanDescriptorSet() = default;

public:
	void CreateDescriptorSet(const vk::ImageView& raytracingImageView);
	void DestroyDescriptorSet();

public:
	const vk::DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	const vk::DescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; }

	void SetVulkanDevice(const VulkanDeviceHandler* vkDevice) { m_VkDevice = vkDevice; }
	void SetVulkanAccelerationStructure(const VulkanAccelerationStructure* vkAccelerationStructure) { m_VkAccelerationStructure = vkAccelerationStructure; }

private:
	const VulkanDeviceHandler* m_VkDevice = nullptr;
	const VulkanAccelerationStructure* m_VkAccelerationStructure = nullptr;

	vk::DescriptorSetLayout m_DescriptorSetLayout;
	vk::DescriptorSet m_DescriptorSet;
	vk::DescriptorPool m_DescriptorPool;

};
