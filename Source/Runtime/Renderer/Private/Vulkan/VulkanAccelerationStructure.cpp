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

/*	vk::BufferCreateInfo BLAS_VoxelBufferInfo(
		vk::BufferCreateFlags(),
		sizeof(MiddlePosition),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	BLAS_VoxelBuffer = m_VkDevice->Raw().createBuffer(BLAS_VoxelBufferInfo);

	VulkanMemoryRequirementsExtended BLAS_VoxelBufferMemoryRequirement = m_VkDevice->FindMemoryRequirement(BLAS_VoxelBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

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

	VulkanMemoryRequirementsExtended memoryRequirement = m_VkDevice->FindMemoryRequirement(BLAS_Buffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

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
*/
	/* BUILD BOTTOM LEVEL */

/*	vk::DeviceAddress BLAS_DeviceAddress = m_VkDevice->Raw().getAccelerationStructureAddressKHR({ BLAS }, *m_Dldi);

	vk::BufferCreateInfo BLAS_ScratchBufferCreateInfo(
		vk::BufferCreateFlags(),
		BLAS_BuildSizesInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
		vk::SharingMode::eExclusive,
		1, &m_VkDevice->GetQueue(VulkanQueueType::Graphic).FamilyIndex
	);

	BLAS_ScratchBuffer = m_VkDevice->Raw().createBuffer(BLAS_ScratchBufferCreateInfo);

	VulkanMemoryRequirementsExtended BLAS_ScratchMemoryIndex = m_VkDevice->FindMemoryRequirement(BLAS_ScratchBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

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
	m_VkDevice->Raw().resetFences(1, &buildingFence);*/

	/* TOP LEVEL */

/*	vk::TransformMatrixKHR identityMatrix(std::array<std::array<float, 4>, 3>(
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

	VulkanMemoryRequirementsExtended TLAS_InstanceMemoryIndex = m_VkDevice->FindMemoryRequirement(TLAS_InstanceBuffer, vk::MemoryPropertyFlagBits::eHostVisible);

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

	VulkanMemoryRequirementsExtended TLAS_MemoryIndex = m_VkDevice->FindMemoryRequirement(TLAS_Buffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

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
*/
	/* BUILD TOP LEVEL */
/*
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

	VulkanMemoryRequirementsExtended TLAS_ScratchMemoryIndex = m_VkDevice->FindMemoryRequirement(TLAS_ScratchBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

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

*/
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