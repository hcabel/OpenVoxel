#include "VulkanRayTracingPipeline.h"
#include "Path.h"
#include "HAL/File.h"
#include "Vulkan/Log.h"
#include "VulkanContext.h"

#include <vulkan/vulkan.hpp>
#include <string.h>

VulkanRayTracingPipeline::VulkanRayTracingPipeline(const vk::DescriptorSetLayout& layout)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		vk::PipelineLayoutCreateFlags(),
		1, &layout,
		0, nullptr
	);
	m_PipelineLayout = VulkanContext::GetDevice().createPipelineLayout(pipelineLayoutInfo);

	// Load shaders
	m_ShaderModules.resize(4);
	m_ShaderModules[0] = CreateShaderModule(Path("/intermediate/Shaders/RayGen.spv"));
	m_ShaderModules[1] = CreateShaderModule(Path("/intermediate/Shaders/Miss.spv"));
	m_ShaderModules[2] = CreateShaderModule(Path("/intermediate/Shaders/Hit.spv"));
	m_ShaderModules[3] = CreateShaderModule(Path("/intermediate/Shaders/Intersection.spv"));

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(4);
	shaderStages[0] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eRaygenKHR,
		m_ShaderModules[0],
		"main",
		nullptr
	);
	shaderStages[1] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eClosestHitKHR,
		m_ShaderModules[1],
		"main",
		nullptr
	);
	shaderStages[2] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eMissKHR,
		m_ShaderModules[2],
		"main",
		nullptr
	);
	shaderStages[3] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eIntersectionKHR,
		m_ShaderModules[3],
		"main",
		nullptr
	);

	m_ShaderGroup.push_back(vk::RayTracingShaderGroupCreateInfoKHR(
		vk::RayTracingShaderGroupTypeKHR::eGeneral,
		0,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR
	));
	m_ShaderGroup.push_back(vk::RayTracingShaderGroupCreateInfoKHR(
		vk::RayTracingShaderGroupTypeKHR::eGeneral,
		2,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR
	));
	m_ShaderGroup.push_back(vk::RayTracingShaderGroupCreateInfoKHR(
		vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup,
		VK_SHADER_UNUSED_KHR,
		1,
		VK_SHADER_UNUSED_KHR,
		3
	));

	vk::RayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo(
		vk::PipelineCreateFlags(),
		static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
		static_cast<uint32_t>(m_ShaderGroup.size()), m_ShaderGroup.data(),
		1,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		m_PipelineLayout
	);
	try {
		auto result = VulkanContext::GetDevice().createRayTracingPipelineKHR(
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			rayTracingPipelineCreateInfo,
			VK_NULL_HANDLE,
			VulkanContext::GetDispatcher()
		);
		*this = std::move(VulkanRayTracingPipeline(result.value, std::move(*this)));
	}
	catch (vk::SystemError& e) {
		VULKAN_LOG(Error, "Failed to create ray tracing pipeline: {:s}", e.what());
	}
}

VulkanRayTracingPipeline::~VulkanRayTracingPipeline()
{
	VulkanContext::GetDevice().destroyPipelineLayout(m_PipelineLayout);
	for (auto& shaderModule : m_ShaderModules)
		VulkanContext::GetDevice().destroyShaderModule(shaderModule);
}

vk::ShaderModule VulkanRayTracingPipeline::CreateShaderModule(const Path& shaderModuleRelativePath)
{
	// Load binary shader file
	auto shaderFile = File::OpenUnique(static_cast<std::string>(Path::GetEngineRootDirectoryPath() + shaderModuleRelativePath), std::ios_base::binary);
	std::string fileContent = shaderFile->ReadAll();
	shaderFile->Close();

	// Create shader module
	vk::ShaderModuleCreateInfo shaderModuleCreateInfo(
		vk::ShaderModuleCreateFlags(),
		fileContent.size(), reinterpret_cast<const uint32_t*>(fileContent.data())
	);
	return (VulkanContext::GetDevice().createShaderModule(shaderModuleCreateInfo));
}
