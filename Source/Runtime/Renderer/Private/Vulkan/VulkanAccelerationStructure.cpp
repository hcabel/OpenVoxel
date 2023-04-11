#include "Vulkan/VulkanAccelerationStructure.h"

void VulkanAccelerationStructure::CreateAccelerationStructure(const vk::CommandBuffer& commandBuffer)
{
	CHECK(m_VkDevice);
	
	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo(
		vk::MemoryAllocateFlagBits::eDeviceAddress
	);

	// Used to wait for the BLAS/TLAS build to complete
	vk::Fence buildingFence = m_VkDevice->Raw().createFence(vk::FenceCreateInfo());

	/* BOTTOM LEVEL */

	vk::BufferCreateInfo BLAS_VoxelBufferInfo(
		vk::BufferCreateFlags(),
		sizeof(MiddlePosition),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	BLAS_VoxelBuffer = m_VkDevice->Raw().createBuffer(BLAS_VoxelBufferInfo);

	MemoryRequirementExtended BLAS_VoxelBufferMemoryRequirement = FindMemoryRequirement(BLAS_VoxelBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

	vk::MemoryAllocateInfo BLAS_VoxelBufferMemoryAllocateInfo(
		BLAS_VoxelBufferMemoryRequirement.size,
		BLAS_VoxelBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	BLAS_VoxelBufferMemory = m_VkDevice->Raw().allocateMemory(BLAS_VoxelBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(BLAS_VoxelBuffer, BLAS_VoxelBufferMemory, 0);

	vk::DeviceAddress BLAS_VoxelBufferAddress = m_VkDevice->Raw().getBufferAddress({ BLAS_VoxelBuffer });

	vk::AccelerationStructureGeometryDataKHR BLAS_GeometryData(
		vk::AccelerationStructureGeometryAabbsDataKHR(
			BLAS_VoxelBufferAddress,
			vk::DeviceSize(sizeof(MiddlePosition))
		)
	);

	vk::AccelerationStructureGeometryKHR BLAS_Geometry(
		vk::GeometryTypeKHR::eAabbs,
		BLAS_GeometryData,
		vk::GeometryFlagBitsKHR::eOpaque
	);

	vk::AccelerationStructureBuildGeometryInfoKHR BLAS_BuildGeometryInfo(
		vk::AccelerationStructureTypeKHR::eBottomLevel,
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		vk::BuildAccelerationStructureModeKHR::eBuild,
		VK_NULL_HANDLE, // Will be resolve later
		VK_NULL_HANDLE, // Will be resolve later
		1, &BLAS_Geometry
	);

	vk::AccelerationStructureBuildSizesInfoKHR BLAS_BuildSizesInfo = m_VkDevice->Raw().getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		BLAS_BuildGeometryInfo,
		1,
		*m_Dldi
	);

	vk::BufferCreateInfo BLAS_BufferInfo(
		vk::BufferCreateFlags(),
		BLAS_BuildSizesInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	BLAS_Buffer = m_VkDevice->Raw().createBuffer(BLAS_BufferInfo);

	MemoryRequirementExtended memoryRequirement = FindMemoryRequirement(BLAS_Buffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo BLAS_MemoryInfo(
		BLAS_BuildSizesInfo.accelerationStructureSize,
		memoryRequirement.MemoryTypeIndex
	);

	BLAS_BufferMemory = m_VkDevice->Raw().allocateMemory(BLAS_MemoryInfo);

	m_VkDevice->Raw().bindBufferMemory(BLAS_Buffer, BLAS_BufferMemory, 0);

	vk::AccelerationStructureCreateInfoKHR BLAS_CreateInfo(
		vk::AccelerationStructureCreateFlagsKHR(),
		BLAS_Buffer,
		0,
		BLAS_BuildSizesInfo.accelerationStructureSize,
		vk::AccelerationStructureTypeKHR::eBottomLevel
	);

	BLAS = m_VkDevice->Raw().createAccelerationStructureKHR(BLAS_CreateInfo, nullptr, *m_Dldi);

	/* BUILD BOTTOM LEVEL */

	vk::DeviceAddress BLAS_DeviceAddress = m_VkDevice->Raw().getAccelerationStructureAddressKHR({ BLAS }, *m_Dldi);

	vk::BufferCreateInfo BLAS_ScratchBufferCreateInfo(
		vk::BufferCreateFlags(),
		BLAS_BuildSizesInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	BLAS_ScratchBuffer = m_VkDevice->Raw().createBuffer(BLAS_ScratchBufferCreateInfo);

	MemoryRequirementExtended BLAS_ScratchMemoryIndex = FindMemoryRequirement(BLAS_ScratchBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo BLAS_ScratchMemoryInfo(
		BLAS_BuildSizesInfo.buildScratchSize,
		BLAS_ScratchMemoryIndex.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	BLAS_ScratchMemory = m_VkDevice->Raw().allocateMemory(BLAS_ScratchMemoryInfo);

	m_VkDevice->Raw().bindBufferMemory(BLAS_ScratchBuffer, BLAS_ScratchMemory, 0);

	vk::DeviceAddress BLAS_ScratchBufferAddress = m_VkDevice->Raw().getBufferAddress({ BLAS_ScratchBuffer });

	BLAS_BuildGeometryInfo.dstAccelerationStructure = BLAS;
	BLAS_BuildGeometryInfo.scratchData.deviceAddress = BLAS_ScratchBufferAddress;

	vk::AccelerationStructureBuildRangeInfoKHR BLAS_BuildRangeInfo(
		1,
		0,
		0,
		1
	);

	commandBuffer.reset();
	vk::CommandBufferBeginInfo BLAS_CommandBufferBeginInfo(
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	);
	commandBuffer.begin(BLAS_CommandBufferBeginInfo);
	const vk::AccelerationStructureBuildRangeInfoKHR* BLAS_BuildRangeInfosPtr =
		&BLAS_BuildRangeInfo;
	commandBuffer.buildAccelerationStructuresKHR(1, &BLAS_BuildGeometryInfo, &BLAS_BuildRangeInfosPtr, *m_Dldi);
	commandBuffer.end();

	vk::SubmitInfo BLAS_SubmitInfo(
		0, nullptr,
		0,
		1, &commandBuffer,
		0, nullptr
	);
	m_VkDevice->GetQueue(VulkanQueueType::Graphic).submit(1, &BLAS_SubmitInfo, buildingFence);
	m_VkDevice->Raw().waitForFences(1, &buildingFence, VK_TRUE, UINT64_MAX);
	m_VkDevice->Raw().resetFences(1, &buildingFence);
	
	/* TOP LEVEL */

	vk::TransformMatrixKHR identityMatrix(std::array<std::array<float, 4>, 3>(
		{
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f}
		}
	));

	vk::AccelerationStructureInstanceKHR TLAS_Instance(
		identityMatrix,
		0,
		0xFF,
		0,
		vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable,
		BLAS_DeviceAddress
	);

	vk::BufferCreateInfo TLAS_InstanceBufferInfo(
		vk::BufferCreateFlags(),
		sizeof(vk::AccelerationStructureInstanceKHR),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	TLAS_InstanceBuffer = m_VkDevice->Raw().createBuffer(TLAS_InstanceBufferInfo);

	MemoryRequirementExtended TLAS_InstanceMemoryIndex = FindMemoryRequirement(TLAS_InstanceBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

	vk::MemoryAllocateInfo TLAS_InstanceMemoryInfo(
		TLAS_InstanceMemoryIndex.size,
		TLAS_InstanceMemoryIndex.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	TLAS_InstanceMemory = m_VkDevice->Raw().allocateMemory(TLAS_InstanceMemoryInfo);

	m_VkDevice->Raw().bindBufferMemory(TLAS_InstanceBuffer, TLAS_InstanceMemory, 0);

	void* TLAS_InstanceBufferPtr = m_VkDevice->Raw().mapMemory(TLAS_InstanceMemory, 0, sizeof(vk::AccelerationStructureInstanceKHR), vk::MemoryMapFlags());

	memcpy(TLAS_InstanceBufferPtr, &TLAS_Instance, sizeof(vk::AccelerationStructureInstanceKHR));

	m_VkDevice->Raw().unmapMemory(TLAS_InstanceMemory);

	vk::BufferDeviceAddressInfo TLAS_InstanceDeviceAddressInfo(
		TLAS_InstanceBuffer
	);

	vk::DeviceAddress TLAS_InstanceBufferAddress = m_VkDevice->Raw().getBufferAddressKHR(TLAS_InstanceDeviceAddressInfo, *m_Dldi);
	
	vk::AccelerationStructureGeometryInstancesDataKHR TLAS_InstanceData(
		VK_FALSE,
		TLAS_InstanceBufferAddress
	);

	vk::AccelerationStructureGeometryKHR TLAS_Geometry(
		vk::GeometryTypeKHR::eInstances,
		TLAS_InstanceData,
		vk::GeometryFlagBitsKHR::eOpaque
	);

	vk::AccelerationStructureBuildGeometryInfoKHR TLAS_BuildGeometryInfo(
		vk::AccelerationStructureTypeKHR::eTopLevel,
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		vk::BuildAccelerationStructureModeKHR::eBuild,
		VK_NULL_HANDLE, // Will be resolve later
		VK_NULL_HANDLE, // Will be resolve later
		1, &TLAS_Geometry
	);

	vk::AccelerationStructureBuildSizesInfoKHR TLAS_BuildSizeInfo = m_VkDevice->Raw().getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		TLAS_BuildGeometryInfo,
		1,
		*m_Dldi
	);

	vk::BufferCreateInfo TLAS_BufferInfo(
		vk::BufferCreateFlags(),
		TLAS_BuildSizeInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	TLAS_Buffer = m_VkDevice->Raw().createBuffer(TLAS_BufferInfo);

	MemoryRequirementExtended TLAS_MemoryIndex = FindMemoryRequirement(TLAS_Buffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo TLAS_BufferMemoryInfo(
		TLAS_MemoryIndex.size,
		TLAS_MemoryIndex.MemoryTypeIndex
	);

	TLAS_BufferMemory = m_VkDevice->Raw().allocateMemory(TLAS_BufferMemoryInfo);

	m_VkDevice->Raw().bindBufferMemory(TLAS_Buffer, TLAS_BufferMemory, 0);

	vk::AccelerationStructureCreateInfoKHR TLAS_CreateInfo(
		vk::AccelerationStructureCreateFlagsKHR(),
		TLAS_Buffer,
		0,
		TLAS_BuildSizeInfo.accelerationStructureSize,
		vk::AccelerationStructureTypeKHR::eTopLevel,
		0
	);

	TLAS = m_VkDevice->Raw().createAccelerationStructureKHR(TLAS_CreateInfo, nullptr, *m_Dldi);

	/* BUILD TOP LEVEL */

	vk::AccelerationStructureDeviceAddressInfoKHR TLAS_DeviceAddressInfo(TLAS);

	vk::DeviceAddress TLAS_DeviceAddress = m_VkDevice->Raw().getAccelerationStructureAddressKHR(TLAS_DeviceAddressInfo, *m_Dldi);
	
	vk::BufferCreateInfo TLAS_ScratchBufferCreateInfo(
		vk::BufferCreateFlags(),
		TLAS_BuildSizeInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	TLAS_ScratchBuffer = m_VkDevice->Raw().createBuffer(TLAS_ScratchBufferCreateInfo);

	MemoryRequirementExtended TLAS_ScratchMemoryIndex = FindMemoryRequirement(TLAS_ScratchBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	vk::MemoryAllocateInfo TLAS_ScratchMemoryInfo(
		TLAS_ScratchMemoryIndex.size,
		TLAS_ScratchMemoryIndex.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	TLAS_ScratchMemory = m_VkDevice->Raw().allocateMemory(TLAS_ScratchMemoryInfo);

	m_VkDevice->Raw().bindBufferMemory(TLAS_ScratchBuffer, TLAS_ScratchMemory, 0);

	vk::DeviceAddress TLAS_ScratchBufferAddress = m_VkDevice->Raw().getBufferAddress({ TLAS_ScratchBuffer });

	TLAS_BuildGeometryInfo.dstAccelerationStructure = TLAS;
	TLAS_BuildGeometryInfo.scratchData.deviceAddress = TLAS_ScratchBufferAddress;

	vk::AccelerationStructureBuildRangeInfoKHR TLAS_BuildRangeInfo(
		1,
		0,
		0,
		1
	);

	commandBuffer.reset();
	commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	commandBuffer.buildAccelerationStructuresKHR(TLAS_BuildGeometryInfo, &TLAS_BuildRangeInfo, *m_Dldi);
	commandBuffer.end();

	vk::SubmitInfo TLAS_SubmitInfo(
		0, nullptr,
		nullptr,
		1, &commandBuffer
	);

	m_VkDevice->GetQueue(VulkanQueueType::Graphic).submit(TLAS_SubmitInfo, buildingFence);
	m_VkDevice->Raw().waitForFences(buildingFence, VK_TRUE, UINT64_MAX);
	m_VkDevice->Raw().resetFences(1, &buildingFence);

	m_VkDevice->Raw().destroyFence(buildingFence);
}

void VulkanAccelerationStructure::DestroyAccelerationStructure()
{
#define _VK_FREE(x) m_VkDevice->Raw().freeMemory(x)
#define _VK_DESTROY_BUFF(x) m_VkDevice->Raw().destroyBuffer(x)

	m_VkDevice->Raw().destroyAccelerationStructureKHR(TLAS, nullptr, *m_Dldi);
	m_VkDevice->Raw().destroyAccelerationStructureKHR(BLAS, nullptr, *m_Dldi);

	_VK_FREE(TLAS_ScratchMemory);
	_VK_DESTROY_BUFF(TLAS_ScratchBuffer);

	_VK_FREE(TLAS_BufferMemory);
	_VK_DESTROY_BUFF(TLAS_Buffer);

	_VK_FREE(TLAS_InstanceMemory);
	_VK_DESTROY_BUFF(TLAS_InstanceBuffer);

	_VK_FREE(BLAS_ScratchMemory);
	_VK_DESTROY_BUFF(BLAS_ScratchBuffer);

	_VK_FREE(BLAS_BufferMemory);
	_VK_DESTROY_BUFF(BLAS_Buffer);

	_VK_FREE(BLAS_VoxelBufferMemory);
	_VK_DESTROY_BUFF(BLAS_VoxelBuffer);
#undef _VK_FREE
#undef _VK_DESTROY_BUFF
}

VulkanAccelerationStructure::MemoryRequirementExtended VulkanAccelerationStructure::FindMemoryRequirement(const vk::Buffer& buffer, vk::MemoryPropertyFlags memoryProperty) const
{
	MemoryRequirementExtended memoryRequirements = static_cast<MemoryRequirementExtended>(m_VkDevice->Raw().getBufferMemoryRequirements(buffer));

	vk::PhysicalDeviceMemoryProperties memoryProperties = m_VkDevice->GetPhysicalDeviceMemoryProperties();

	memoryRequirements.MemoryTypeIndex = -1;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (memoryRequirements.memoryTypeBits & (1 << i))
		{
			if (memoryProperties.memoryTypes[i].propertyFlags & memoryProperty)
			{
				memoryRequirements.MemoryTypeIndex = i;
				break;
			}
		}
	}

	return (memoryRequirements);
}
