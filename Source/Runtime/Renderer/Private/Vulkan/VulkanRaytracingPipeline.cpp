#include "Vulkan/VulkanRayTracingPipeline.h"
#include "Vulkan/VulkanDeviceHandler.h"

#include <fstream>

void VulkanRayTracingPipeline::CreateRayTracingPipeline(const vk::DescriptorSetLayout& vkDescriptorLayout)
{
	// Pipeline Layout

	vk::DescriptorSetLayout descriptorSetLayouts = vkDescriptorLayout;
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		vk::PipelineLayoutCreateFlags(),
		1, &descriptorSetLayouts,
		0, nullptr
	);

	m_PipelineLayout = m_VkDevice->Raw().createPipelineLayout(pipelineLayoutInfo, nullptr, *m_Dldi);

	// Load shaders
	m_ShaderModules[RayTracingShaderType::RayGeneration].push_back(CreateShaderModule("C:/MyProject/OpenVoxel/intermediate/Shaders/RayGen.spv"));
	m_ShaderModules[RayTracingShaderType::Miss].push_back(CreateShaderModule("C:/MyProject/OpenVoxel/intermediate/Shaders/Miss.spv"));
	m_ShaderModules[RayTracingShaderType::ClossestHit].push_back(CreateShaderModule("C:/MyProject/OpenVoxel/intermediate/Shaders/Hit.spv"));
	m_ShaderModules[RayTracingShaderType::Intersection].push_back(CreateShaderModule("C:/MyProject/OpenVoxel/intermediate/Shaders/Intersection.spv"));

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	shaderStages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eRaygenKHR,
			m_ShaderModules[RayTracingShaderType::RayGeneration][0],
			"main",
			nullptr
		)
	);
	shaderStages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eClosestHitKHR,
			m_ShaderModules[RayTracingShaderType::ClossestHit][0],
			"main",
			nullptr
		)
	);
	shaderStages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eMissKHR,
			m_ShaderModules[RayTracingShaderType::Miss][0],
			"main",
			nullptr
		)
	);
	shaderStages.push_back(
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eIntersectionKHR,
			m_ShaderModules[RayTracingShaderType::Intersection][0],
			"main",
			nullptr
		)
	);

	m_ShaderGroups.clear();
	m_ShaderGroups.push_back(
		vk::RayTracingShaderGroupCreateInfoKHR(
			vk::RayTracingShaderGroupTypeKHR::eGeneral,
			0,
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR
		)
	);
	m_ShaderGroups.push_back(
		vk::RayTracingShaderGroupCreateInfoKHR(
			vk::RayTracingShaderGroupTypeKHR::eGeneral,
			2,
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR
		)
	);
	m_ShaderGroups.push_back(
		vk::RayTracingShaderGroupCreateInfoKHR(
			vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup,
			VK_SHADER_UNUSED_KHR,
			1,
			VK_SHADER_UNUSED_KHR,
			3
		)
	);

	vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo(
		vk::PipelineCreateFlags(),
		shaderStages.size(), shaderStages.data(),
		m_ShaderGroups.size(), m_ShaderGroups.data(),
		10,
		nullptr,
		nullptr,
		nullptr,
		m_PipelineLayout,
		VK_NULL_HANDLE,
		0
	);

	m_Pipeline = m_VkDevice->Raw().createRayTracingPipelineKHR(VK_NULL_HANDLE, VK_NULL_HANDLE, pipelineCreateInfo, nullptr, *m_Dldi).value;
}

void VulkanRayTracingPipeline::DestroyRayTracingPipeline()
{
	for (int i = 0; i < RayTracingShaderType::COUNT; ++i)
	{
		for (auto& shaderModule : m_ShaderModules[i])
		{
			m_VkDevice->Raw().destroyShaderModule(shaderModule);
		}
	}

	m_VkDevice->Raw().destroyPipelineLayout(m_PipelineLayout);
	m_VkDevice->Raw().destroyPipeline(m_Pipeline);
}

vk::ShaderModule VulkanRayTracingPipeline::CreateShaderModule(const char* path) const
{
	// load path to text
	std::ifstream shaderFile(path, std::ios::ate | std::ios::binary);
	if (!shaderFile.is_open())
	{
		OV_LOG(LogVulkan, Fatal, "Unable to load shader \"{:s}\"", path);
		return {};
	}

	std::streamsize fileSize = shaderFile.tellg();
	shaderFile.seekg(0, std::ios::beg);
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	shaderFile.read((char*)buffer.data(), fileSize);

	shaderFile.close();

	vk::ShaderModuleCreateInfo createInfo(
		vk::ShaderModuleCreateFlags(),
		buffer.size() * sizeof(uint32_t), reinterpret_cast<const uint32_t*>(buffer.data())
	);

	return m_VkDevice->Raw().createShaderModule(createInfo);
}

/* RAYTRACING SHADER TYPE */

vk::ShaderStageFlagBits RayTracingShaderType::ToShaderStageFlagBits(Type rayTracingShaderType)
{
	switch (rayTracingShaderType)
	{
	case RayTracingShaderType::RayGeneration:
		return (vk::ShaderStageFlagBits::eRaygenKHR);
	case RayTracingShaderType::Miss:
		return (vk::ShaderStageFlagBits::eMissKHR);
	case RayTracingShaderType::ClossestHit:
		return (vk::ShaderStageFlagBits::eClosestHitKHR);
	case RayTracingShaderType::Intersection:
		return (vk::ShaderStageFlagBits::eIntersectionKHR);
	}
	return (vk::ShaderStageFlagBits::eAll);
}
