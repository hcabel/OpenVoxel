#pragma once

#include "Vulkan_API.h"
#include "VulkanBuffer.h"

#include <vulkan/vulkan.hpp>
#include <vector>

class Path;

/**
 * Wrapper around the vk::Pipeline ptr.
 *
 * To create it just call the constructor with the vk::DescriptorSetLayout as parameter.
 *
 * Here is how the whole proccess of creation should go:
 * 1- The class is automatically created with the default constructor.
 * 2- The user calls the constructor with the vk::DescriptorSetLayout as parameter.
 *   Which will create the pipeline, this pipeline can't be set by this class because it's a private member of vk::Pipeline.
 *   To assign it anyway we call a private constructor that will recreate the object and then move it to the current object members into the new instance.
 *   By doing so we have an instance which has the pipeline and all the other VulkanRayTracingPipeline members set. (moved to avoid copying)
 *   Now we need to move this object to this so the User doesn't realize that the object was recreated.
 *   That's where the = operator comes in, it will move the object to the this ptr.
 * 3- You can destroy the object by assigning it to nullptr or by calling the destructor.
 */
class VULKAN_API VulkanRayTracingPipeline : public vk::Pipeline
{

protected:
	// m_pipeline is a private member of vk::Pipeline, so to assigned it I recreate the object and then I move it to the current object
	VulkanRayTracingPipeline(vk::Pipeline rayTracingPipeline, VulkanRayTracingPipeline&& self);

public:
	VulkanRayTracingPipeline() // Default constructor
		: vk::Pipeline(nullptr),
		m_PipelineLayout(nullptr),
		m_ShaderModules(0),
		m_ShaderGroup(0),
		m_ShaderBindingTableBuffer(),
		m_RaygenSbt(),
		m_MissSbt(),
		m_HitSbt(),
		m_CallableSbt()
	{}
	VulkanRayTracingPipeline(std::nullptr_t) : VulkanRayTracingPipeline() {} // default constructor 2 (for null assignment)
	VulkanRayTracingPipeline(const vk::DescriptorSetLayout& layout); // The real constructor (the one who create the pipeline)
	~VulkanRayTracingPipeline();

	VulkanRayTracingPipeline& operator=(VulkanRayTracingPipeline&& self) noexcept
	{
		static_cast<vk::Pipeline*>(this)->operator=(self);
		static_cast<vk::Pipeline&&>(self).operator=(VK_NULL_HANDLE);

		m_PipelineLayout = std::move(self.m_PipelineLayout);
		self.m_PipelineLayout = nullptr;

		m_ShaderBindingTableBuffer = std::move(self.m_ShaderBindingTableBuffer);
		self.m_ShaderBindingTableBuffer = nullptr;

		m_RaygenSbt = std::move(self.m_RaygenSbt);
		m_MissSbt = std::move(self.m_MissSbt);
		m_HitSbt = std::move(self.m_HitSbt);
		m_CallableSbt = std::move(self.m_CallableSbt);

		m_ShaderModules = std::move(self.m_ShaderModules);
		m_ShaderGroup = std::move(self.m_ShaderGroup);
		return *this;
	}

public:
	__forceinline const vk::PipelineLayout GetLayout() const { return m_PipelineLayout; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetRaygenSbt() const { return m_RaygenSbt; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetMissSbt() const { return m_MissSbt; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetHitSbt() const { return m_HitSbt; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetCallableSbt() const { return m_CallableSbt; }

protected:
	vk::ShaderModule CreateShaderModule(const Path& shaderModuleRelativePath);

protected:
	vk::PipelineLayout m_PipelineLayout;
	std::vector<vk::ShaderModule> m_ShaderModules;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_ShaderGroup;

	VulkanBuffer m_ShaderBindingTableBuffer;
	vk::StridedDeviceAddressRegionKHR m_RaygenSbt;
	vk::StridedDeviceAddressRegionKHR m_MissSbt;
	vk::StridedDeviceAddressRegionKHR m_HitSbt;
	vk::StridedDeviceAddressRegionKHR m_CallableSbt;
};
