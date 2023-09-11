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
	m_ShaderModules[0] = CreateShaderModule(Path("intermediate/Shaders/RayGen.spv"));
	m_ShaderModules[1] = CreateShaderModule(Path("intermediate/Shaders/Miss.spv"));
	m_ShaderModules[2] = CreateShaderModule(Path("intermediate/Shaders/Hit.spv"));
	m_ShaderModules[3] = CreateShaderModule(Path("intermediate/Shaders/Intersection.spv"));

	std::array<vk::PipelineShaderStageCreateInfo, 4> shaderStages;
	shaderStages[0] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eRaygenKHR,
		m_ShaderModules[0],
		"main",
		nullptr
	);
	shaderStages[1] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eMissKHR,
		m_ShaderModules[1],
		"main",
		nullptr
	);
	shaderStages[2] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eClosestHitKHR,
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

	m_ShaderGroup.push_back(vk::RayTracingShaderGroupCreateInfoKHR( // Raygen group
		vk::RayTracingShaderGroupTypeKHR::eGeneral,
		0,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR
	));
	m_ShaderGroup.push_back(vk::RayTracingShaderGroupCreateInfoKHR( // Miss group
		vk::RayTracingShaderGroupTypeKHR::eGeneral,
		1,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR
	));
	m_ShaderGroup.push_back(vk::RayTracingShaderGroupCreateInfoKHR( // Hit group (with closest hit and intersection shaders)
		vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup,
		VK_SHADER_UNUSED_KHR,
		2,
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

	/* CREATE SBT */

	auto align_up = [](uint32_t a, size_t x) { return ((x + ((a) - 1)) & ~(a - 1)); };
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties
		= VulkanContext::GetPhysicalDevice()
		.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>()
		.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

	uint32_t handleSize = rtProperties.shaderGroupHandleSize;
	uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = align_up(handleSize, rtProperties.shaderGroupHandleAlignment);

	m_RaygenSbt.stride = align_up(handleSizeAligned, baseAlignment);
	m_RaygenSbt.size = m_RaygenSbt.stride;

	m_MissSbt.stride = align_up(handleSizeAligned, baseAlignment);
	m_MissSbt.size = align_up(m_MissSbt.stride, baseAlignment);

	m_HitSbt.stride = align_up(handleSizeAligned, baseAlignment);
	m_HitSbt.size = align_up(m_HitSbt.stride, baseAlignment);

	// Not using callable shaders yet
	// m_CallableSbt.stride = ...
	// m_CallableSbt.size = ...

	constexpr uint8_t shaderCount = 3;
	uint32_t dataSize = handleSize * shaderCount;
	std::vector<uint8_t> handles(dataSize);
	VulkanContext::GetDevice().getRayTracingShaderGroupHandlesKHR(
		*this,
		0, shaderCount,
		dataSize,
		handles.data(),
		VulkanContext::GetDispatcher()
	);

	m_ShaderBindingTableBuffer = VulkanBuffer(
		m_RaygenSbt.size + m_MissSbt.size + m_HitSbt.size + m_CallableSbt.size,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,

	m_RaygenSbt.deviceAddress = m_ShaderBindingTableBuffer.GetDeviceAddress();
	m_MissSbt.deviceAddress = m_ShaderBindingTableBuffer.GetDeviceAddress() + m_RaygenSbt.size;
	m_HitSbt.deviceAddress = m_ShaderBindingTableBuffer.GetDeviceAddress() + m_RaygenSbt.size + m_MissSbt.size;
	m_CallableSbt.deviceAddress = 0; // Not using callable shaders yet

	void* data = m_ShaderBindingTableBuffer.Map();

	void* pData = data; // Create a pointer that will move around (still in the boundary of the buffer though)
	memcpy(pData, handles.data(), dataSize); // Copy Raygen group handle

	pData = &data + m_RaygenSbt.size; // Move the pointer after raygen
	memcpy(pData, handles.data() + handleSize, dataSize); // Copy Miss group handle

	pData = &data + m_RaygenSbt.size + m_MissSbt.size; // Move the pointer after miss
	memcpy(pData, handles.data() + handleSize * 2, dataSize); // Copy Hit group handle

	m_ShaderBindingTableBuffer.Unmap();
}

VulkanRayTracingPipeline::~VulkanRayTracingPipeline()
{
	m_ShaderBindingTableBuffer.~VulkanBuffer();

	VulkanContext::GetDevice().destroyPipelineLayout(m_PipelineLayout);
	for (auto& shaderModule : m_ShaderModules)
		VulkanContext::GetDevice().destroyShaderModule(shaderModule);
}

vk::ShaderModule VulkanRayTracingPipeline::CreateShaderModule(const Path& shaderModuleRelativePath)
{
	// Load binary shader file
	auto shaderFile = File::Open(
		(Path::GetEngineRootDirectoryPath() + shaderModuleRelativePath).GetPath(),
		std::ios::in | std::ios::binary
	);
	std::string fileContent = shaderFile->ReadAll();
	shaderFile->Close();

	// Create shader module
	vk::ShaderModuleCreateInfo shaderModuleCreateInfo(
		vk::ShaderModuleCreateFlags(),
		fileContent.size(), reinterpret_cast<const uint32_t*>(fileContent.data())
	);
	return (VulkanContext::GetDevice().createShaderModule(shaderModuleCreateInfo));
}
