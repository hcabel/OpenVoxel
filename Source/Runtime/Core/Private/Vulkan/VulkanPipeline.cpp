#include "Vulkan/VulkanPipeline.h"
#include "FileSystem.h"

void VulkanPipeline::CreatePipeline(const Vulkan::DeviceBundle& device)
{
	constexpr const char* rayGenShaderFilePath = "C:/MyProject WIP/OpenVoxel/intermediate/Shaders/RayGen.spv";
	constexpr const char* missShaderFilePath = "C:/MyProject WIP/OpenVoxel/intermediate/Shaders/Miss.spv";
	constexpr const char* hitShaderFilePath = "C:/MyProject WIP/OpenVoxel/intermediate/Shaders/Hit.spv";

	GraphicsPipelineInBundle inBundle;
	inBundle.device = device;
	inBundle.rayGenShaderFilePath = rayGenShaderFilePath;

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
		CreateShaderStage(rayGenShaderFilePath, device, vk::ShaderStageFlagBits::eRaygenKHR),
		CreateShaderStage(missShaderFilePath, device, vk::ShaderStageFlagBits::eMissKHR),
		CreateShaderStage(hitShaderFilePath, device, vk::ShaderStageFlagBits::eClosestHitKHR)
	};

	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups = {
		vk::RayTracingShaderGroupCreateInfoKHR(
			vk::RayTracingShaderGroupTypeKHR::eGeneral,
			0,
			2, // closest hit shader index
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR
		),
	};

	vk::RayTracingPipelineCreateInfoKHR createPipelineInfos = vk::RayTracingPipelineCreateInfoKHR(
		vk::PipelineCreateFlags(),
		static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
		static_cast<uint32_t>(shaderGroups.size()), shaderGroups.data(),
		1,
		nullptr,
		nullptr,
		NULL,
		nullptr
	);
}

void	VulkanPipeline::BuildAccelerationStructure(const Vulkan::DeviceBundle& device)
{
	// Build BLAS
}

VulkanPipeline::GraphicsPipelineOutBundle VulkanPipeline::CreateGraphicsPipeline(const GraphicsPipelineInBundle& inBundle)
{
	vk::GraphicsPipelineCreateInfo createInfo = {};
	createInfo.flags = vk::PipelineCreateFlags();

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
		CreateShaderStage(inBundle.rayGenShaderFilePath, inBundle.device, vk::ShaderStageFlagBits::eRaygenKHR)
	};

	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups = {
		vk::RayTracingShaderGroupCreateInfoKHR(
			vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup,
			0,
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR,
			VK_SHADER_UNUSED_KHR
		)
	};

	vk::RayTracingPipelineCreateInfoKHR createInfos = vk::RayTracingPipelineCreateInfoKHR(
		vk::PipelineCreateFlags(),
		static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
		static_cast<uint32_t>(shaderGroups.size()), shaderGroups.data(),
		0,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		0
	);

	try {
	//	auto t = inBundle.device.Logical.createRayTracingPipelineKHR(nullptr, nullptr, createInfos, nullptr, nullptr);
	}
	catch (vk::SystemError& e) {
		OV_LOG(Error, LogVulkan, "Failed to create ray tracing pipeline: {:s}", e.what());
	}

	GraphicsPipelineOutBundle outBundle;
	return (outBundle);
}

vk::PipelineShaderStageCreateInfo VulkanPipeline::CreateShaderStage(const char* shaderFilePath, const Vulkan::DeviceBundle& device, vk::ShaderStageFlagBits stage)
{
	FileHandler shaderFile(shaderFilePath);
	auto shaderModule = CreateShaderModule(shaderFile.ReadFile(), device);
	if (!shaderModule)
		OV_LOG(Fatal, LogVulkan, "Failed to create shader module");
	OV_LOG(Verbose, LogVulkan, "\t\"{:s}\"", shaderFilePath);
	vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = {
		vk::PipelineShaderStageCreateFlags(),
		stage,
		shaderModule,
		"main"
	};
	return (shaderStageCreateInfo);
}

vk::ShaderModule VulkanPipeline::CreateShaderModule(const std::vector<char>& code, const Vulkan::DeviceBundle& device)
{
	OV_LOG(Verbose, LogVulkan, "Code size: {:d}", code.size());
	vk::ShaderModuleCreateInfo createInfo(
		vk::ShaderModuleCreateFlags(),
		code.size(),
		reinterpret_cast<const uint32_t*>(code.data())
	);

	try {
		return device.Logical.createShaderModule(createInfo);
	}
	catch (vk::SystemError& e) {
		OV_LOG(Error, LogVulkan, "Failed to create shader module: {:s}", e.what());
	}
	return (nullptr);
}