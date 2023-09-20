#include "VulkanRaytracingPipeline.h"
#include "Path.h"
#include "HAL/File.h"
#include "Vulkan/Log.h"
#include "VulkanContext.h"

#include <vulkan/vulkan.hpp>
#include <string.h>

#pragma region Lifecycle
VulkanRaytracingPipeline VulkanRaytracingPipeline::Create(const vk::DescriptorSetLayout& layout)
{
	// Create pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		vk::PipelineLayoutCreateFlags(),
		1, &layout,
		0, nullptr
	);
	vk::PipelineLayout pipelineLayout = VulkanContext::GetDevice().createPipelineLayout(pipelineLayoutInfo);

	// Load shaders
	ShaderModulesArray shaderModules;
	shaderModules[0] = CreateShaderModule(Path("intermediate/Shaders/RayGen.spv"));
	shaderModules[1] = CreateShaderModule(Path("intermediate/Shaders/Miss.spv"));
	shaderModules[2] = CreateShaderModule(Path("intermediate/Shaders/Hit.spv"));
	shaderModules[3] = CreateShaderModule(Path("intermediate/Shaders/Intersection.spv"));

	// Set shader stages
	std::array<vk::PipelineShaderStageCreateInfo, 4> shaderStages;
	shaderStages[0] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eRaygenKHR,
		shaderModules[0],
		"main",
		nullptr
	);
	shaderStages[1] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eMissKHR,
		shaderModules[1],
		"main",
		nullptr
	);
	shaderStages[2] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eClosestHitKHR,
		shaderModules[2],
		"main",
		nullptr
	);
	shaderStages[3] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eIntersectionKHR,
		shaderModules[3],
		"main",
		nullptr
	);

	// Set shader groups
	std::array<vk::RayTracingShaderGroupCreateInfoKHR, 3> shaderGroups;
	shaderGroups[0] = vk::RayTracingShaderGroupCreateInfoKHR( // Raygen group
		vk::RayTracingShaderGroupTypeKHR::eGeneral,
		0,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR
	);
	shaderGroups[1] = vk::RayTracingShaderGroupCreateInfoKHR( // Miss group
		vk::RayTracingShaderGroupTypeKHR::eGeneral,
		1,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR
	);
	shaderGroups[2] = vk::RayTracingShaderGroupCreateInfoKHR( // Hit group (with closest hit and intersection shaders)
		vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup,
		VK_SHADER_UNUSED_KHR,
		2,
		VK_SHADER_UNUSED_KHR,
		3
	);

	// Create the pipeline
	vk::RayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo(
		vk::PipelineCreateFlags(),
		static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
		static_cast<uint32_t>(shaderGroups.size()), shaderGroups.data(),
		20,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		pipelineLayout,
		VK_NULL_HANDLE,
		0
	);
	vk::Pipeline raytracingPipeline;
	try {
		raytracingPipeline = VulkanContext::GetDevice().createRayTracingPipelineKHR(
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			rayTracingPipelineCreateInfo,
			VK_NULL_HANDLE,
			VulkanContext::GetDispatcher()
		).value;
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

	vk::StridedDeviceAddressRegionKHR rGenSbtEntry;
	rGenSbtEntry.stride = align_up(handleSizeAligned, baseAlignment);
	rGenSbtEntry.size = rGenSbtEntry.stride;

	vk::StridedDeviceAddressRegionKHR missSbtEntry;
	missSbtEntry.stride = align_up(handleSizeAligned, baseAlignment);
	missSbtEntry.size = align_up(missSbtEntry.stride, baseAlignment);

	vk::StridedDeviceAddressRegionKHR hitSbtEntry;
	hitSbtEntry.stride = align_up(handleSizeAligned, baseAlignment);
	hitSbtEntry.size = align_up(hitSbtEntry.stride, baseAlignment);

	// Calculate the total size of the shader group handles
	constexpr uint8_t shaderCount = 3;
	uint32_t shaderGroupsHandleBufferSize = handleSizeAligned * shaderCount;

	// Recover the shader group handles in a temporary buffer
	uint8_t* shaderGroupHandles = new uint8_t[shaderGroupsHandleBufferSize];
	VulkanContext::GetDevice().getRayTracingShaderGroupHandlesKHR(
		raytracingPipeline,
		0, shaderCount,
		shaderGroupsHandleBufferSize,
		shaderGroupHandles,
		VulkanContext::GetDispatcher()
	);

	// Create the vk::buffer that will hold the whole SBT
	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo(
		vk::MemoryAllocateFlagBits::eDeviceAddress
	);
	VulkanBuffer shaderBindingTable = VulkanBuffer::Create(
		rGenSbtEntry.size + missSbtEntry.size + hitSbtEntry.size,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		&memoryAllocateFlagsInfo
	);

	rGenSbtEntry.deviceAddress = shaderBindingTable.GetDeviceAddress();
	missSbtEntry.deviceAddress = shaderBindingTable.GetDeviceAddress() + rGenSbtEntry.size;
	hitSbtEntry.deviceAddress = shaderBindingTable.GetDeviceAddress() + rGenSbtEntry.size + missSbtEntry.size;

	uint8_t* data = reinterpret_cast<uint8_t*>(shaderBindingTable.Map());

	// Copy RayGen shader handle
	uint8_t* pData = data;
	uint8_t* pHandle = shaderGroupHandles;
	memcpy(pData, pHandle, handleSize);

	// Copy Miss shader handle
	pData = data + rGenSbtEntry.size;
	pHandle = shaderGroupHandles + handleSize;
	memcpy(pData, pHandle, handleSize);

	// Copy Hit shader handle
	pData = data + rGenSbtEntry.size + missSbtEntry.size;
	pHandle = shaderGroupHandles + handleSize * 2;
	memcpy(pData, pHandle, handleSize);

	shaderBindingTable.Unmap();

	delete shaderGroupHandles;

	// Move whole the data to the VulkanRaytracingPipeline wrapper
	return (VulkanRaytracingPipeline(
		std::move(raytracingPipeline),
		std::move(pipelineLayout),
		std::move(shaderModules),
		std::move(shaderBindingTable),
		std::move(rGenSbtEntry),
		std::move(missSbtEntry),
		std::move(hitSbtEntry),
		vk::StridedDeviceAddressRegionKHR() // Not using Callable shaders yet
	));
}

VulkanRaytracingPipeline::VulkanRaytracingPipeline(
	vk::Pipeline&& pipeline,
	vk::PipelineLayout&& pipelineLayout,
	ShaderModulesArray&& shaderModules,
	VulkanBuffer&& shaderBindingTableBuffer,
	vk::StridedDeviceAddressRegionKHR&& raygenSbt,
	vk::StridedDeviceAddressRegionKHR&& missSbt,
	vk::StridedDeviceAddressRegionKHR&& hitSbt,
	vk::StridedDeviceAddressRegionKHR&& callableSbt
) noexcept
	: vk::Pipeline(static_cast<VkPipeline&&>(pipeline)),
	m_PipelineLayout(std::move(pipelineLayout)),
	m_ShaderModules(std::move(shaderModules)),
	m_ShaderBindingTable(std::move(shaderBindingTableBuffer)),
	m_RaygenSbt(std::move(raygenSbt)),
	m_MissSbt(std::move(missSbt)),
	m_HitSbt(std::move(hitSbt)),
	m_CallableSbt(std::move(callableSbt))
{
	pipeline.vk::Pipeline::operator=(VK_NULL_HANDLE);
	pipelineLayout = nullptr;
	shaderModules = ShaderModulesArray();
	shaderBindingTableBuffer = nullptr;
	raygenSbt = vk::StridedDeviceAddressRegionKHR();
	missSbt = vk::StridedDeviceAddressRegionKHR();
	hitSbt = vk::StridedDeviceAddressRegionKHR();
	callableSbt = vk::StridedDeviceAddressRegionKHR();
}

VulkanRaytracingPipeline::~VulkanRaytracingPipeline()
{
	if (IsValid())
	{
		VulkanContext::GetDevice().destroyPipelineLayout(m_PipelineLayout);

		for (auto& shaderModule : m_ShaderModules)
			VulkanContext::GetDevice().destroyShaderModule(shaderModule);

		VulkanContext::GetDevice().destroyPipeline(*this);
	}
}

VulkanRaytracingPipeline::VulkanRaytracingPipeline(VulkanRaytracingPipeline&& other) noexcept
	: vk::Pipeline(static_cast<VkPipeline&&>(other)),
	m_PipelineLayout(std::move(other.m_PipelineLayout)),
	m_ShaderModules(std::move(other.m_ShaderModules)),
	m_ShaderBindingTable(std::move(other.m_ShaderBindingTable)),
	m_RaygenSbt(std::move(other.m_RaygenSbt)),
	m_MissSbt(std::move(other.m_MissSbt)),
	m_HitSbt(std::move(other.m_HitSbt)),
	m_CallableSbt(std::move(other.m_CallableSbt))
{
	other.vk::Pipeline::operator=(VK_NULL_HANDLE);
	other.m_PipelineLayout = nullptr;
	other.m_ShaderModules = ShaderModulesArray();
	other.m_ShaderBindingTable = nullptr;
	other.m_RaygenSbt = vk::StridedDeviceAddressRegionKHR();
	other.m_MissSbt = vk::StridedDeviceAddressRegionKHR();
	other.m_HitSbt = vk::StridedDeviceAddressRegionKHR();
	other.m_CallableSbt = vk::StridedDeviceAddressRegionKHR();
}

VulkanRaytracingPipeline& VulkanRaytracingPipeline::operator=(VulkanRaytracingPipeline&& rhs) noexcept
{
	if (this != &rhs) // If not the same object
	{
		vk::Pipeline::operator=(static_cast<VkPipeline&&>(rhs));
		m_PipelineLayout = std::move(rhs.m_PipelineLayout);
		m_ShaderModules = std::move(rhs.m_ShaderModules);
		m_ShaderBindingTable = std::move(rhs.m_ShaderBindingTable);
		m_RaygenSbt = std::move(rhs.m_RaygenSbt);
		m_MissSbt = std::move(rhs.m_MissSbt);
		m_HitSbt = std::move(rhs.m_HitSbt);
		m_CallableSbt = std::move(rhs.m_CallableSbt);

		rhs.vk::Pipeline::operator=(VK_NULL_HANDLE);
		rhs.m_PipelineLayout = nullptr;
		rhs.m_ShaderModules = ShaderModulesArray();
		rhs.m_ShaderBindingTable = nullptr;
		rhs.m_RaygenSbt = vk::StridedDeviceAddressRegionKHR();
		rhs.m_MissSbt = vk::StridedDeviceAddressRegionKHR();
		rhs.m_HitSbt = vk::StridedDeviceAddressRegionKHR();
		rhs.m_CallableSbt = vk::StridedDeviceAddressRegionKHR();
	}
	return *this;
}
#pragma endregion Lifecycle

vk::ShaderModule VulkanRaytracingPipeline::CreateShaderModule(const Path& shaderModuleRelativePath)
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
