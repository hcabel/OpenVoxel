#include "Vulkan/VulkanShaderBindingTable.h"
#include "Vulkan/VulkanDeviceHandler.h"
#include "Vulkan/VulkanRaytracingPipeline.h"

#include <fstream>

void VulkanShaderBindingTable::CreateShaderBindingTable()
{
	CHECK(m_VkDevice && m_VkPipeline);

	auto align_up = [](uint32_t a, size_t x) { return ((x + ((a) - 1)) & ~(a - 1)); };

	auto rtProperties = m_VkDevice->GetRaytracingProperties();

	uint32_t handleSize = rtProperties.shaderGroupHandleSize;
	uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = align_up(handleSize, rtProperties.shaderGroupHandleAlignment);

	constexpr uint32_t hitShaderCount = 1;
	constexpr uint32_t missShaderCount = 1;
	constexpr uint32_t handleCount = 1 /* ray gen */ + hitShaderCount + missShaderCount;

	m_RaygenShaderBindingTable.stride = align_up(handleSizeAligned, baseAlignment);
	m_RaygenShaderBindingTable.size = m_RaygenShaderBindingTable.stride;

	m_MissShaderBindingTable.stride = align_up(handleSizeAligned, baseAlignment);
	m_MissShaderBindingTable.size = align_up(missShaderCount * m_MissShaderBindingTable.stride, baseAlignment);

	m_HitShaderBindingTable.stride = align_up(handleSizeAligned, baseAlignment);
	m_HitShaderBindingTable.size = align_up(hitShaderCount * m_HitShaderBindingTable.stride, baseAlignment);

	uint32_t dataSize = handleCount * handleSize;
	std::vector<uint8_t> handles(dataSize);
	m_VkDevice->Raw().getRayTracingShaderGroupHandlesKHR(
		m_VkPipeline->Raw(),
		0, handleCount,
		dataSize,
		handles.data(),
		*m_Dldi
	);

	vk::DeviceSize sbtSize = m_RaygenShaderBindingTable.size + m_MissShaderBindingTable.size + m_HitShaderBindingTable.size + m_CallableShaderBindingTable.size;
	vk::BufferCreateInfo sbtBufferInfo(
		vk::BufferCreateFlags(),
		sbtSize,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR
		| vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eShaderDeviceAddress
	);

	m_SbtBuffer = m_VkDevice->Raw().createBuffer(sbtBufferInfo);

	VulkanMemoryRequirementsExtended sbtMemoryRequirements =
		m_VkDevice->FindMemoryRequirement(
			m_SbtBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo(
		vk::MemoryAllocateFlagBits::eDeviceAddress
	);
	vk::MemoryAllocateInfo sbtMemoryAllocateInfo(
		sbtMemoryRequirements.size,
		sbtMemoryRequirements.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	m_SbtBufferMemory = m_VkDevice->Raw().allocateMemory(sbtMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(m_SbtBuffer, m_SbtBufferMemory, 0);

	vk::DeviceAddress sbtDeviceAddress = m_VkDevice->Raw().getBufferAddress({ m_SbtBuffer });
	m_RaygenShaderBindingTable.deviceAddress = sbtDeviceAddress;
	m_MissShaderBindingTable.deviceAddress = sbtDeviceAddress + m_RaygenShaderBindingTable.size;
	m_HitShaderBindingTable.deviceAddress = sbtDeviceAddress + m_RaygenShaderBindingTable.size + m_MissShaderBindingTable.size;

	void* data;
	m_VkDevice->Raw().mapMemory(m_SbtBufferMemory, 0, sbtSize, vk::MemoryMapFlags(), &data);

	auto getHandle = [handles, handleSize](int i) { return handles.data() + i * handleSize; };

	auto* pSBTBuffer = reinterpret_cast<uint8_t*>(data);

	uint8_t* pData{ nullptr };
	uint32_t handleIndex = 0;
	pData = pSBTBuffer;

	// Ray gen
	memcpy(pData, getHandle(handleIndex), handleSize);
	handleIndex++;

	// Hit
	pData =
		pSBTBuffer /* start */
		+ m_RaygenShaderBindingTable.size /* raygen */;
	memcpy(pData, getHandle(handleIndex), handleSize);
	handleIndex++;

	// Miss
	pData =
		pSBTBuffer /* Start */
		+ m_RaygenShaderBindingTable.size /* raygen */
		+ m_MissShaderBindingTable.size; /* Hit shaders */
	memcpy(pData, getHandle(handleIndex), handleSize);
	handleIndex++;

	m_VkDevice->Raw().unmapMemory(m_SbtBufferMemory);
}

void VulkanShaderBindingTable::DestroyShaderBindingTable()
{
	m_VkDevice->Raw().freeMemory(m_SbtBufferMemory);
	m_VkDevice->Raw().destroyBuffer(m_SbtBuffer);
}
