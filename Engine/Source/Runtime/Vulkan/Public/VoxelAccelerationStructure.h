#pragma once

#include "Vulkan_API.h"
#include "VulkanBuffer.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VULKAN_API VoxelAccelerationStructure
{

#pragma region Lifecycle
public:
	/**
	 * @brief Create a new voxel based Acceleration Structure
	 *
	 * @return The new VoxelAccelerationStructure wrapper instance (not allocated on the heap)
	 */
	static VoxelAccelerationStructure Create();
	/**
	 * @brief Default constructor, will do has little has possible, everything will be set to default/nullptr values
	 */
	VoxelAccelerationStructure()
		: m_IsRebuildNeeded(false)
	{}
	/**
	 * @brief Allow nullptr assignment, same has using the default constructor
	 */
	VoxelAccelerationStructure(std::nullptr_t) : VoxelAccelerationStructure() {}
	~VoxelAccelerationStructure();
	/**
	 * @brief Move constructor, will create a new instance with the data from the other instance, and un-validate the other instance
	 *
	 * @param other The VoxelAccelerationStructure to move from
	 */
	VoxelAccelerationStructure(VoxelAccelerationStructure&& other) noexcept;
	/**
	 * @brief Move assignment operator, will move the data from the rhs to the lhs, and un-validate the rhs
	 *
	 * @param rhs The VoxelAccelerationStructure to move
	 */
	VoxelAccelerationStructure& operator=(VoxelAccelerationStructure&& rhs) noexcept;

protected:
	/**
	 * @note Should only be called by @ref Create
	 * @brief Wrapper constructor, will take all related data and wrap it in a VoxelAccelerationStructure
	 */
	VoxelAccelerationStructure(VulkanBuffer&& geometryDataBuffer) noexcept
		: m_IsRebuildNeeded(false),
		m_GeometryDataBuffer(std::move(geometryDataBuffer))
	{}
#pragma endregion Lifecycle

public:
	/**
	 * @brief Check if the VoxelAccelerationStructure is valid
	 */
	__forceinline operator bool() const { return m_BottomLevelAccelerationStructure && m_TopLevelAccelerationStructure; }

public:
	void AddVoxel();
	void Rebuild();

public:
	__forceinline bool IsValid() const { return (this->operator bool()); }
	__forceinline bool IsRebuildNeeded() const { return m_IsRebuildNeeded; }
	__forceinline const vk::AccelerationStructureKHR& GetBLAS() const { return m_BottomLevelAccelerationStructure; }
	__forceinline const vk::AccelerationStructureKHR& GetTLAS() const { return m_TopLevelAccelerationStructure; }

protected:
	struct VoxelInstanceInfo
	{
		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Scale;

		VoxelInstanceInfo(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
			: Position(position), Rotation(rotation), Scale(scale)
		{}
	};

protected:
	bool m_IsRebuildNeeded;

	VulkanBuffer m_GeometryDataBuffer;
	VulkanBuffer m_BlasScratchBuffer;
	VulkanBuffer m_TlasInstanceBuffer;
	VulkanBuffer m_TlasScratchBuffer;

	vk::AccelerationStructureKHR m_BottomLevelAccelerationStructure;
	vk::AccelerationStructureKHR m_TopLevelAccelerationStructure;

	std::vector<VoxelInstanceInfo> m_VoxelInstances;

};
