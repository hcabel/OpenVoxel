#include "VoxelAccelerationStructure.h"
#include "VulkanContext.h"

#include <glm/gtc/matrix_transform.hpp>

#pragma region Lifecycle
VoxelAccelerationStructure VoxelAccelerationStructure::Create()
{
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

	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo(
		vk::MemoryAllocateFlagBits::eDeviceAddress
	);
	VulkanBuffer geometryDataBuffer = VulkanBuffer::Create(
		sizeof(Aabb),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible,
		&memoryAllocateFlagsInfo
	);

	void* data = geometryDataBuffer.Map();
	memcpy(data, aabbs.data(), sizeof(Aabb));
	geometryDataBuffer.Unmap();

	return (VoxelAccelerationStructure(
		std::move(geometryDataBuffer))
	);
}

VoxelAccelerationStructure::~VoxelAccelerationStructure()
{
	if (IsValid())
	{
		VulkanContext::GetDevice().destroyAccelerationStructureKHR(m_BottomLevelAccelerationStructure, nullptr, VulkanContext::GetDispatcher());
		VulkanContext::GetDevice().destroyAccelerationStructureKHR(m_TopLevelAccelerationStructure, nullptr, VulkanContext::GetDispatcher());
	}
}

VoxelAccelerationStructure::VoxelAccelerationStructure(VoxelAccelerationStructure &&other) noexcept
	: m_IsRebuildNeeded(std::move(other.m_IsRebuildNeeded)),
	m_GeometryDataBuffer(std::move(other.m_GeometryDataBuffer)),
	m_BlasScratchBuffer(std::move(other.m_BlasScratchBuffer)),
	m_TlasInstanceBuffer(std::move(other.m_TlasInstanceBuffer)),
	m_TlasScratchBuffer(std::move(other.m_TlasScratchBuffer)),
	m_BottomLevelAccelerationStructure(std::move(other.m_BottomLevelAccelerationStructure)),
	m_TopLevelAccelerationStructure(std::move(other.m_TopLevelAccelerationStructure)),
	m_VoxelInstances(std::move(other.m_VoxelInstances))
{
	other.m_IsRebuildNeeded = false;
	other.m_GeometryDataBuffer = nullptr;
	other.m_BlasScratchBuffer = nullptr;
	other.m_TlasInstanceBuffer = nullptr;
	other.m_TlasScratchBuffer = nullptr;
	other.m_BottomLevelAccelerationStructure = nullptr;
	other.m_TopLevelAccelerationStructure = nullptr;
	other.m_VoxelInstances.clear();
}

VoxelAccelerationStructure &VoxelAccelerationStructure::operator=(VoxelAccelerationStructure &&rhs) noexcept
{
	if (this != &rhs) // If not the same object
	{
		m_IsRebuildNeeded = std::move(rhs.m_IsRebuildNeeded);
		m_GeometryDataBuffer = std::move(rhs.m_GeometryDataBuffer);
		m_BlasScratchBuffer = std::move(rhs.m_BlasScratchBuffer);
		m_TlasInstanceBuffer = std::move(rhs.m_TlasInstanceBuffer);
		m_TlasScratchBuffer = std::move(rhs.m_TlasScratchBuffer);
		m_BottomLevelAccelerationStructure = std::move(rhs.m_BottomLevelAccelerationStructure);
		m_TopLevelAccelerationStructure = std::move(rhs.m_TopLevelAccelerationStructure);
		m_VoxelInstances = std::move(rhs.m_VoxelInstances);

		rhs.m_IsRebuildNeeded = false;
		rhs.m_GeometryDataBuffer = nullptr;
		rhs.m_BlasScratchBuffer = nullptr;
		rhs.m_TlasInstanceBuffer = nullptr;
		rhs.m_TlasScratchBuffer = nullptr;
		rhs.m_BottomLevelAccelerationStructure = nullptr;
		rhs.m_TopLevelAccelerationStructure = nullptr;
		rhs.m_VoxelInstances.clear();
	}

	return (*this);
}
#pragma endregion Lifecycle

// LINK error when using glm::vec3 as parameter
// void VoxelAccelerationStructure::AddVoxel(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
void VoxelAccelerationStructure::AddVoxel()
{
	m_IsRebuildNeeded = true;

	VoxelInstanceInfo voxelInstanceInfo(
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f)
	);

	m_VoxelInstances.push_back(voxelInstanceInfo);
}

void VoxelAccelerationStructure::Rebuild()
{
	vk::AccelerationStructureGeometryAabbsDataKHR blasGeometryAabbsData(
		m_GeometryDataBuffer.GetDeviceAddress(),
		sizeof(vk::AabbPositionsKHR)
	);

	vk::AccelerationStructureGeometryKHR blasGeometry(
		vk::GeometryTypeKHR::eAabbs,
		blasGeometryAabbsData,
		vk::GeometryFlagBitsKHR::eOpaque
	);

	vk::AccelerationStructureBuildRangeInfoKHR aabbBuildOffsetInfo(
		1, // primitiveCount
		0, // primitiveOffset
		0, // firstVertex
		0  // transformOffset
	);

	/* CREATE BLAS */

	vk::AccelerationStructureBuildGeometryInfoKHR blasBuildGeometryInfo(
		vk::AccelerationStructureTypeKHR::eBottomLevel,
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		vk::BuildAccelerationStructureModeKHR::eBuild,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE, // (dstAccelerationStructure) Will be set later
		1, &blasGeometry
	);

	vk::AccelerationStructureBuildSizesInfoKHR blasBuildSizesInfo
		= VulkanContext::GetDevice().getAccelerationStructureBuildSizesKHR(
			vk::AccelerationStructureBuildTypeKHR::eDevice,
			blasBuildGeometryInfo,
			aabbBuildOffsetInfo.primitiveCount,
			VulkanContext::GetDispatcher()
		);

	vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo(
		vk::MemoryAllocateFlagBits::eDeviceAddress
	);
	m_BlasScratchBuffer = VulkanBuffer::Create(
		blasBuildSizesInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer
		| vk::BufferUsageFlagBits::eShaderDeviceAddress
		| vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		&memoryAllocateFlagsInfo
	);

	vk::AccelerationStructureCreateInfoKHR blasCreateInfo(
		vk::AccelerationStructureCreateFlagsKHR(),
		m_BlasScratchBuffer,
		0,
		blasBuildSizesInfo.buildScratchSize,
		vk::AccelerationStructureTypeKHR::eBottomLevel
	);
	m_BottomLevelAccelerationStructure = VulkanContext::GetDevice().createAccelerationStructureKHR(blasCreateInfo, nullptr, VulkanContext::GetDispatcher());

	/* BUILD BLAS */

	blasBuildGeometryInfo.dstAccelerationStructure = m_BottomLevelAccelerationStructure; // (dstAccelerationStructure) Set here
	blasBuildGeometryInfo.scratchData.deviceAddress = m_BlasScratchBuffer.GetDeviceAddress();

	const vk::AccelerationStructureBuildRangeInfoKHR* blasBuildRangeInfo = &aabbBuildOffsetInfo;

	VulkanContext::GetDevice().SubmitOneTimeCommandBuffer(
		VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
		[&blasBuildRangeInfo, &blasBuildGeometryInfo](vk::CommandBuffer commandBuffer)
		{
			commandBuffer.buildAccelerationStructuresKHR(1, &blasBuildGeometryInfo, &blasBuildRangeInfo, VulkanContext::GetDispatcher());
		}
	);

	/* CREATE TLAS */

	std::vector<vk::AccelerationStructureInstanceKHR> tlasInstances;
	tlasInstances.reserve(m_VoxelInstances.size());
	vk::DeviceAddress blasAddress = VulkanContext::GetDevice().getAccelerationStructureAddressKHR({ m_BottomLevelAccelerationStructure }, VulkanContext::GetDispatcher());
	for (uint32_t i = 0; i < m_VoxelInstances.size(); i++)
	{
		// Create the vk::TransformMatrixKHR from the 3 glm::vec3 (position, rotation, scale)
		vk::TransformMatrixKHR transformMatrixKHR(
			std::array<std::array<float, 4>, 3> {
				std::array<float, 4>({ m_VoxelInstances[i].Scale.x, 0.0f, 0.0f, m_VoxelInstances[i].Position.x }),
				std::array<float, 4>({ 0.0f, m_VoxelInstances[i].Scale.y, 0.0f, m_VoxelInstances[i].Position.y }),
				std::array<float, 4>({ 0.0f, 0.0f, m_VoxelInstances[i].Scale.z, m_VoxelInstances[i].Position.z })
			}
		);

		vk::AccelerationStructureInstanceKHR tlasInstance(
			transformMatrixKHR,
			0,
			0xFF,
			0,
			vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,
			blasAddress
		);
		tlasInstances.push_back(tlasInstance);
	}

	m_TlasInstanceBuffer = VulkanBuffer::Create(
		sizeof(vk::AccelerationStructureInstanceKHR) * tlasInstances.size(),
		vk::BufferUsageFlagBits::eShaderDeviceAddress
		| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		&memoryAllocateFlagsInfo
	);
	void* data = m_TlasInstanceBuffer.Map();
	memcpy(data, tlasInstances.data(), sizeof(vk::AccelerationStructureInstanceKHR) * tlasInstances.size());
	m_TlasInstanceBuffer.Unmap();

	vk::AccelerationStructureGeometryInstancesDataKHR tlasGeometryInstancesData;
	tlasGeometryInstancesData.data.deviceAddress = m_TlasInstanceBuffer.GetDeviceAddress();

	vk::AccelerationStructureGeometryKHR tlasGeometry(
		vk::GeometryTypeKHR::eInstances,
		tlasGeometryInstancesData
	);

	vk::AccelerationStructureBuildGeometryInfoKHR tlasBuildGeometryInfo(
		vk::AccelerationStructureTypeKHR::eTopLevel,
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
		vk::BuildAccelerationStructureModeKHR::eBuild,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE, // (dstAccelerationStructure) Will be set later
		1, &tlasGeometry
	);
	vk::AccelerationStructureBuildSizesInfoKHR tlasBuildSizesInfo
		= VulkanContext::GetDevice().getAccelerationStructureBuildSizesKHR(
			vk::AccelerationStructureBuildTypeKHR::eDevice,
			tlasBuildGeometryInfo,
			1,
			VulkanContext::GetDispatcher()
		);

	m_TlasScratchBuffer = VulkanBuffer::Create(
		tlasBuildSizesInfo.buildScratchSize,
		vk::BufferUsageFlagBits::eStorageBuffer
		| vk::BufferUsageFlagBits::eShaderDeviceAddress
		| vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
		| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		&memoryAllocateFlagsInfo
	);

	vk::AccelerationStructureCreateInfoKHR tlasCreateInfo(
		vk::AccelerationStructureCreateFlagsKHR(),
		m_TlasScratchBuffer,
		0,
		tlasBuildSizesInfo.buildScratchSize,
		vk::AccelerationStructureTypeKHR::eTopLevel
	);
	m_TopLevelAccelerationStructure = VulkanContext::GetDevice().createAccelerationStructureKHR(tlasCreateInfo, nullptr, VulkanContext::GetDispatcher());

	/* BUILD TLAS */

	tlasBuildGeometryInfo.dstAccelerationStructure = m_TopLevelAccelerationStructure; // (dstAccelerationStructure) Set here
	tlasBuildGeometryInfo.scratchData.deviceAddress = m_TlasScratchBuffer.GetDeviceAddress();

	vk::AccelerationStructureBuildRangeInfoKHR tlasBuildOffsetInfo(
		tlasInstances.size(), // primitiveCount
		0, // primitiveOffset
		0, // firstVertex
		0  // transformOffset
	);
	const vk::AccelerationStructureBuildRangeInfoKHR* tlasBuildRangeInfo = &tlasBuildOffsetInfo;

	VulkanContext::GetDevice().SubmitOneTimeCommandBuffer(
		VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
		[&tlasBuildRangeInfo, &tlasBuildGeometryInfo](vk::CommandBuffer commandBuffer)
		{
			commandBuffer.buildAccelerationStructuresKHR(1, &tlasBuildGeometryInfo, &tlasBuildRangeInfo, VulkanContext::GetDispatcher());
		}
	);

	m_IsRebuildNeeded = false;
}
