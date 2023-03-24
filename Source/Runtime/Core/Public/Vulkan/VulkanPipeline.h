#pragma once

#include "CoreModule.h"
#include "Vulkan/VulkanImpl.h"

class CORE_API VulkanPipeline
{
public:
	struct GraphicsPipelineInBundle
	{
		Vulkan::DeviceBundle device;
		const char* rayGenShaderFilePath;
		const char* missShaderFilePath;
		const char* hitShaderFilePath;

		GraphicsPipelineInBundle()
			: device(), rayGenShaderFilePath(""), missShaderFilePath(""), hitShaderFilePath("")
		{}
	};

	struct GraphicsPipelineOutBundle
	{
	};

public:
	VulkanPipeline() = delete;
	~VulkanPipeline() = delete;

public:
	static void CreatePipeline(const Vulkan::DeviceBundle& device);

	static void BuildAccelerationStructure(const Vulkan::DeviceBundle& device);

private:
	static GraphicsPipelineOutBundle CreateGraphicsPipeline(const GraphicsPipelineInBundle& inBundle);
	static vk::PipelineShaderStageCreateInfo CreateShaderStage(const char* shaderFilePath, const Vulkan::DeviceBundle& device, vk::ShaderStageFlagBits stage);
	static vk::ShaderModule CreateShaderModule(const std::vector<char>& code, const Vulkan::DeviceBundle& device);

};
