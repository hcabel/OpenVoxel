#pragma once

#include "Vulkan_API.h"
#include "VulkanBuffer.h"

#include <vulkan/vulkan.hpp>

class Path;

class VULKAN_API VulkanRaytracingPipeline : public vk::Pipeline
{
protected:
	using ShaderModulesArray = std::array<vk::ShaderModule, 4>;

#pragma region Lifecycle
public:
	/**
	 * @brief Create a ray tracing pipeline and the SBT (Shader Binding Table) for it.
	 *
	 * @param layout The descriptor set layout used by the pipeline
	 * @return A new VulkanRaytracingPipeline wrapper instance (not allocated on the heap)
	 */
	static VulkanRaytracingPipeline Create(const vk::DescriptorSetLayout& layout);
	/**
	 * @brief default constructor, will not create the pipeline, see @ref Create for that
	 */
	VulkanRaytracingPipeline() = default;
	/**
	 * @brief null pointer constructor, same has using the default constructor
	 */
	VulkanRaytracingPipeline(std::nullptr_t) : VulkanRaytracingPipeline() {}
	~VulkanRaytracingPipeline();

	/**
	 * @brief Move constructor, will create a new instance with the data from the other instance, and un-validate the other instance
	 *
	 * @param other The VulkanRaytracingPipeline to move from
	 */
	VulkanRaytracingPipeline(VulkanRaytracingPipeline&& other) noexcept;
	/**
	 * @brief Move assignment operator, will move the data from the rhs to the lhs, and un-validate the rhs
	 *
	 * @param rhs The VulkanRaytracingPipeline to move
	 * @return The new VulkanRaytracingPipeline
	 */
	VulkanRaytracingPipeline& operator=(VulkanRaytracingPipeline&& rhs) noexcept;

protected:
	/**
	 * @note Should only be called by @ref Create
	 * @brief Wrapper constructor, will take all related data and wrap it in a VulkanRaytracingPipeline
	 */
	VulkanRaytracingPipeline(
		vk::Pipeline&& pipeline,
		vk::PipelineLayout&& pipelineLayout,
		ShaderModulesArray&& shaderModules,
		VulkanBuffer&& shaderBindingTableBuffer,
		vk::StridedDeviceAddressRegionKHR&& raygenSbt,
		vk::StridedDeviceAddressRegionKHR&& missSbt,
		vk::StridedDeviceAddressRegionKHR&& hitSbt,
		vk::StridedDeviceAddressRegionKHR&& callableSbt
	) noexcept;
#pragma endregion Lifecycle

public:
	/**
	 * @brief Check is the pipeline has been created or not
	 */
	__forceinline operator bool() const { return (vk::Pipeline::operator bool()); }

public:
	__forceinline bool IsValid() const { return (this->operator bool()); }
	__forceinline const vk::PipelineLayout GetLayout() const { return m_PipelineLayout; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetRaygenSbt() const { return m_RaygenSbt; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetMissSbt() const { return m_MissSbt; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetHitSbt() const { return m_HitSbt; }
	__forceinline const vk::StridedDeviceAddressRegionKHR& GetCallableSbt() const { return m_CallableSbt; }

protected:
	static vk::ShaderModule CreateShaderModule(const Path& shaderModuleRelativePath);

protected:
	vk::PipelineLayout m_PipelineLayout;
	ShaderModulesArray m_ShaderModules;

	VulkanBuffer m_ShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_RaygenSbt;
	vk::StridedDeviceAddressRegionKHR m_MissSbt;
	vk::StridedDeviceAddressRegionKHR m_HitSbt;
	vk::StridedDeviceAddressRegionKHR m_CallableSbt;
};
