#pragma once

#include "Renderer_API.h"
#include "Vulkan/VulkanUtils.h"

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanDeviceHandler;

namespace RayTracingShaderType
{
	enum Type : uint8_t
	{
		RayGeneration = 0,
		Miss = 1,
		ClossestHit = 2,
		Intersection = 3,

		COUNT = 4
	};

	vk::ShaderStageFlagBits ToShaderStageFlagBits(Type rayTracingShaderType);
}

class RENDERER_API VulkanRayTracingPipeline final
{

public:
	VulkanRayTracingPipeline() = default;
	VulkanRayTracingPipeline(const VulkanDeviceHandler* vkDevice)
		: m_VkDevice(vkDevice)
	{}
	~VulkanRayTracingPipeline() = default;

	operator vk::Pipeline() const { return Raw(); }
	operator vk::PipelineLayout() const { return GetPipelineLayout(); }

public:
	void CreateRayTracingPipeline(const vk::DescriptorSetLayout& vkDescriptorLayout);
	void DestroyRayTracingPipeline();

private:
	vk::ShaderModule CreateShaderModule(const char* path) const;

public:
	vk::Pipeline Raw() const { return m_Pipeline; }
	vk::PipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

	uint32_t GetShaderGroupCount() const { return static_cast<uint32_t>(m_ShaderGroups.size()); }

	void SetVulkanDevice(const VulkanDeviceHandler* vkDevice) { m_VkDevice = vkDevice; }
	void SetDispatchLoaderDynamic(const vk::DispatchLoaderDynamic* dldi) { m_Dldi = dldi; }

private:
	const VulkanDeviceHandler* m_VkDevice = nullptr;
	const vk::DispatchLoaderDynamic* m_Dldi = nullptr;

	vk::PipelineLayout m_PipelineLayout;
	vk::Pipeline m_Pipeline;

	/* Store all the loaded shaders */
	std::array<std::vector<vk::ShaderModule>, RayTracingShaderType::COUNT> m_ShaderModules;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_ShaderGroups;
};
