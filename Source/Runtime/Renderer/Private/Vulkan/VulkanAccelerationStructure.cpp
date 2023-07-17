#include "Vulkan/VulkanAccelerationStructure.h"

void VulkanAccelerationStructure::CreateAccelerationStructure(const vk::CommandBuffer& commandBuffer)
{
	CHECK(m_VkDevice);

	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo(
		vk::MemoryAllocateFlagBits::eDeviceAddress
	);

	// Used to wait for the BLAS/TLAS build to complete
	vk::Fence buildingFence = m_VkDevice->Raw().createFence(vk::FenceCreateInfo());

	/* CUBES */

	struct Aabb
	{
		glm::vec3 Min;
		glm::vec3 Max;

		Aabb(glm::vec3&& min, glm::vec3&& max)
			: Min(std::move(min)), Max(std::move(max))
		{}
	};

	std::vector<Aabb> aabbs;
	aabbs.push_back(
		Aabb(
			glm::vec3(-0.5, -0.5, -0.5),
			glm::vec3(0.5, 0.5, 0.5)
		)
	);

	vk::BufferCreateInfo aabbBufferInfo(
		vk::BufferCreateFlags(),
		sizeof(Aabb),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	m_AabbBuffer = m_VkDevice->Raw().createBuffer(aabbBufferInfo);

	VulkanMemoryRequirementsExtended aabbBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(m_AabbBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

	vk::MemoryAllocateInfo aabbBufferMemoryAllocateInfo(
		aabbBufferMemoryRequirement.size,
		aabbBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	m_AabbBufferMemory = m_VkDevice->Raw().allocateMemory(aabbBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(m_AabbBuffer, m_AabbBufferMemory, 0);

	void* aabbBufferMemoryData = m_VkDevice->Raw().mapMemory(m_AabbBufferMemory, 0, sizeof(Aabb), vk::MemoryMapFlags());
	memcpy(aabbBufferMemoryData, aabbs.data(), sizeof(Aabb));

	/* BOTTOM LEVEL ACCELERATION STRUCTURE */
	uint32_t blasSize = 0;
	uint32_t blasScratchBufferMaxSize = 0;

	vk::DeviceAddress aabbBufferAddress = m_VkDevice->Raw().getBufferAddress({ m_AabbBuffer }, *m_Dldi);

	vk::AccelerationStructureGeometryAabbsDataKHR aabbGeometryData(
		aabbBufferAddress,
		vk::DeviceSize(sizeof(Aabb))
	);

	vk::AccelerationStructureGeometryKHR aabbGeometry(
		vk::GeometryTypeKHR::eAabbs,
		aabbGeometryData,
		vk::GeometryFlagBitsKHR::eOpaque
	);

	vk::AccelerationStructureBuildRangeInfoKHR aabbBuildOffsetInfo(
		aabbs.size(), // primitiveCount
		0, // primitiveOffset
		0, // firstVertex
		0  // transformOffset
	);

	/* BLAS BUILD INFO */

	vk::AccelerationStructureBuildGeometryInfoKHR blasBuildGeometryInfo(
		vk::AccelerationStructureTypeKHR::eBottomLevel,
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		vk::BuildAccelerationStructureModeKHR::eBuild,
		VK_NULL_HANDLE, // Will be resolve later
		VK_NULL_HANDLE, // Will be resolve later
		aabbs.size(), &aabbGeometry
	);
	vk::AccelerationStructureBuildRangeInfoKHR blasBuildOffsetInfo = aabbBuildOffsetInfo;

	vk::AccelerationStructureBuildSizesInfoKHR blasBuildSizesInfo =
		m_VkDevice->Raw().getAccelerationStructureBuildSizesKHR(
			vk::AccelerationStructureBuildTypeKHR::eDevice,
			blasBuildGeometryInfo,
			blasBuildOffsetInfo.primitiveCount,
			*m_Dldi
		);
	blasSize = blasBuildSizesInfo.accelerationStructureSize;
	blasScratchBufferMaxSize = blasBuildSizesInfo.buildScratchSize;

	/* BLAS SCRATCH BUFFER */

	vk::BufferCreateInfo blasScratchBufferInfo(
		vk::BufferCreateFlags(),
		blasScratchBufferMaxSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	vk::Buffer blasScratchBuffer = m_VkDevice->Raw().createBuffer(blasScratchBufferInfo);

	VulkanMemoryRequirementsExtended blasScratchBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(blasScratchBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo blasScratchBufferMemoryAllocateInfo(
		blasScratchBufferMemoryRequirement.size,
		blasScratchBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	vk::DeviceMemory blasScratchBufferMemory = m_VkDevice->Raw().allocateMemory(blasScratchBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(blasScratchBuffer, blasScratchBufferMemory, 0);

	vk::DeviceAddress blasScratchBufferAddress = m_VkDevice->Raw().getBufferAddress({ blasScratchBuffer }, *m_Dldi);

	vk::AccelerationStructureCreateInfoKHR blasCreateInfo;
	blasCreateInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	blasCreateInfo.size = blasSize;

	vk::BufferCreateInfo blasBufferInfo(
		vk::BufferCreateFlags(),
		blasSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	m_BlasBuffer = m_VkDevice->Raw().createBuffer(blasBufferInfo);

	VulkanMemoryRequirementsExtended blasBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(m_BlasBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo blasBufferMemoryAllocateInfo(
		blasBufferMemoryRequirement.size,
		blasBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	m_BlasBufferMemory = m_VkDevice->Raw().allocateMemory(blasBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(m_BlasBuffer, m_BlasBufferMemory, 0);

	vk::DeviceAddress blasBufferAddress = m_VkDevice->Raw().getBufferAddress({ m_BlasBuffer }, *m_Dldi);

	blasCreateInfo.buffer = m_BlasBuffer;

	m_Blas = m_VkDevice->Raw().createAccelerationStructureKHR(blasCreateInfo, nullptr, *m_Dldi);

	/* BLAS BUILD */

	blasBuildGeometryInfo.dstAccelerationStructure = m_Blas;
	blasBuildGeometryInfo.scratchData.deviceAddress = blasScratchBufferAddress;

	const vk::AccelerationStructureBuildRangeInfoKHR* blasBuildRangeInfosPtr = &blasBuildOffsetInfo;

	commandBuffer.reset();
	commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	commandBuffer.buildAccelerationStructuresKHR(1, &blasBuildGeometryInfo, &blasBuildRangeInfosPtr, *m_Dldi);
	commandBuffer.end();

	vk::SubmitInfo blasSubmitInfo(
		0, nullptr,
		nullptr,
		1, &commandBuffer
	);

	m_VkDevice->GetQueue(VulkanQueueType::Graphic).submit(1, &blasSubmitInfo, nullptr);
	m_VkDevice->GetQueue(VulkanQueueType::Graphic).waitIdle();

	m_VkDevice->Raw().destroyBuffer(blasScratchBuffer);
	m_VkDevice->Raw().freeMemory(blasScratchBufferMemory);

	/* TOP LEVEL ACCELERATION STRUCTURE */

	vk::DeviceAddress blasAddress = m_VkDevice->Raw().getAccelerationStructureAddressKHR({ m_Blas }, *m_Dldi);

	vk::TransformMatrixKHR identityMatrix(std::array<std::array<float, 4>, 3>(
		{
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f}
		}
	));

	vk::AccelerationStructureInstanceKHR tlasInstance(
		identityMatrix,
		0,
		0xFF,
		0,
		vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,
		blasAddress
	);

	vk::BufferCreateInfo tlasInstanceBufferInfo(
		vk::BufferCreateFlags(),
		sizeof(vk::AccelerationStructureInstanceKHR),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	m_TlasInstanceBuffer = m_VkDevice->Raw().createBuffer(tlasInstanceBufferInfo);

	VulkanMemoryRequirementsExtended tlasInstanceBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(m_TlasInstanceBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

	vk::MemoryAllocateInfo tlasInstanceBufferMemoryAllocateInfo(
		tlasInstanceBufferMemoryRequirement.size,
		tlasInstanceBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	m_TlasInstanceBufferMemory = m_VkDevice->Raw().allocateMemory(tlasInstanceBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(m_TlasInstanceBuffer, m_TlasInstanceBufferMemory, 0);

	void* data = m_VkDevice->Raw().mapMemory(m_TlasInstanceBufferMemory, 0, sizeof(vk::AccelerationStructureInstanceKHR), vk::MemoryMapFlags());
	memcpy(data, &tlasInstance, sizeof(vk::AccelerationStructureInstanceKHR));

	m_VkDevice->Raw().unmapMemory(m_TlasInstanceBufferMemory);

	vk::DeviceAddress tlasInstanceBufferAddress = m_VkDevice->Raw().getBufferAddress({ m_TlasInstanceBuffer }, *m_Dldi);

	vk::AccelerationStructureGeometryInstancesDataKHR tlasInstanceData;
	tlasInstanceData.data.deviceAddress = tlasInstanceBufferAddress;

	vk::AccelerationStructureGeometryKHR tlasGeometry(
		vk::GeometryTypeKHR::eInstances,
		tlasInstanceData
	);

	vk::AccelerationStructureBuildGeometryInfoKHR tlasBuildGeometryInfo(
		vk::AccelerationStructureTypeKHR::eTopLevel,
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		vk::BuildAccelerationStructureModeKHR::eBuild,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		1, &tlasGeometry
	);

	vk::AccelerationStructureBuildSizesInfoKHR tlasBuildSizeInfo = m_VkDevice->Raw().getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		tlasBuildGeometryInfo,
		1,
		*m_Dldi
	);

	vk::AccelerationStructureCreateInfoKHR tlasCreateInfo;
	tlasCreateInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	tlasCreateInfo.size = tlasBuildSizeInfo.accelerationStructureSize;

	vk::BufferCreateInfo tlasBufferInfo(
		vk::BufferCreateFlags(),
		tlasBuildSizeInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	m_TlasBuffer = m_VkDevice->Raw().createBuffer(tlasBufferInfo);

	VulkanMemoryRequirementsExtended tlasBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(m_TlasBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo tlasBufferMemoryAllocateInfo(
		tlasBufferMemoryRequirement.size,
		tlasBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	m_TlasBufferMemory = m_VkDevice->Raw().allocateMemory(tlasBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(m_TlasBuffer, m_TlasBufferMemory, 0);

	tlasCreateInfo.buffer = m_TlasBuffer;

	m_Tlas = m_VkDevice->Raw().createAccelerationStructureKHR(tlasCreateInfo, nullptr, *m_Dldi);

	/* TLAS SCRACH BUFFER */

	vk::BufferCreateInfo tlasScratchBufferInfo(
		vk::BufferCreateFlags(),
		tlasBuildSizeInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	vk::Buffer tlasScratchBuffer = m_VkDevice->Raw().createBuffer(tlasScratchBufferInfo);

	VulkanMemoryRequirementsExtended tlasScratchBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(tlasScratchBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::MemoryAllocateInfo tlasScratchBufferMemoryAllocateInfo(
		tlasScratchBufferMemoryRequirement.size,
		tlasScratchBufferMemoryRequirement.MemoryTypeIndex,
		&memoryAllocateFlagsInfo
	);

	vk::DeviceMemory tlasScratchBufferMemory = m_VkDevice->Raw().allocateMemory(tlasScratchBufferMemoryAllocateInfo);

	m_VkDevice->Raw().bindBufferMemory(tlasScratchBuffer, tlasScratchBufferMemory, 0);

	/* TLAS BUILD */

	vk::DeviceAddress tlasScratchBufferAddress = m_VkDevice->Raw().getBufferAddress({ tlasScratchBuffer }, *m_Dldi);

	tlasBuildGeometryInfo.dstAccelerationStructure = m_Tlas;
	tlasBuildGeometryInfo.scratchData.deviceAddress = tlasScratchBufferAddress;

	vk::AccelerationStructureBuildRangeInfoKHR tlasBuildRangeInfo(1, 0, 0, 0);
	const vk::AccelerationStructureBuildRangeInfoKHR* tlaspBuildRangeInfo = &tlasBuildRangeInfo;

	commandBuffer.reset();
	commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	commandBuffer.buildAccelerationStructuresKHR(1, &tlasBuildGeometryInfo, &tlaspBuildRangeInfo, *m_Dldi);
	commandBuffer.end();

	vk::SubmitInfo tlasBuildSubmitInfo;
	tlasBuildSubmitInfo.commandBufferCount = 1;
	tlasBuildSubmitInfo.pCommandBuffers = &commandBuffer;

	m_VkDevice->GetQueue(VulkanQueueType::Graphic).submit(1, &tlasBuildSubmitInfo, VK_NULL_HANDLE);
	m_VkDevice->GetQueue(VulkanQueueType::Graphic).waitIdle();

	m_VkDevice->Raw().freeMemory(tlasScratchBufferMemory, nullptr);
	m_VkDevice->Raw().destroyBuffer(tlasScratchBuffer, nullptr);

	m_VkDevice->Raw().destroyFence(buildingFence);
}

void VulkanAccelerationStructure::DestroyAccelerationStructure()
{
#define _VK_FREE(x) m_VkDevice->Raw().freeMemory(x)
#define _VK_DESTROY_BUFF(x) m_VkDevice->Raw().destroyBuffer(x)

	m_VkDevice->Raw().destroyAccelerationStructureKHR(m_Tlas, nullptr, *m_Dldi);
	m_VkDevice->Raw().destroyAccelerationStructureKHR(m_Blas, nullptr, *m_Dldi);

	_VK_FREE(m_AabbBufferMemory);
	_VK_DESTROY_BUFF(m_AabbBuffer);

	_VK_FREE(m_BlasBufferMemory);
	_VK_DESTROY_BUFF(m_BlasBuffer);

	_VK_FREE(m_TlasInstanceBufferMemory);
	_VK_DESTROY_BUFF(m_TlasInstanceBuffer);

	_VK_FREE(m_TlasBufferMemory);
	_VK_DESTROY_BUFF(m_TlasBuffer);

#undef _VK_FREE
#undef _VK_DESTROY_BUFF
}
