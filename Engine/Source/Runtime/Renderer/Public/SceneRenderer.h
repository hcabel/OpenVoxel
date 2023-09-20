#pragma once

#include "Renderer_API.h"
#include "VulkanRayTracingPipeline.h"
#include "VoxelAccelerationStructure.h"

#include <vulkan/vulkan.hpp>
#include <vector>

class RENDERER_API SceneRenderer
{
public:
	static SceneRenderer Create(uint32_t frameCount);

protected:
	SceneRenderer(
		VulkanRaytracingPipeline&& pipeline,
		VoxelAccelerationStructure&& accelerationStructure,
		vk::DescriptorPool&& descriptorPool,
		std::vector<vk::DescriptorSetLayout>&& descriptorSetLayouts,
		std::vector<vk::DescriptorSet>&& descriptorSets
	)
		: m_Pipeline(std::move(pipeline)),
		m_AccelerationStructure(std::move(accelerationStructure)),
		m_DescriptorPool(std::move(descriptorPool)),
		m_DescriptorSetLayouts(std::move(descriptorSetLayouts)),
		m_DescriptorSets(std::move(descriptorSets))
	{}

public:
	SceneRenderer() = default;
	~SceneRenderer();

	SceneRenderer(SceneRenderer&& other) noexcept
		: m_Pipeline(std::move(other.m_Pipeline)),
		m_AccelerationStructure(std::move(other.m_AccelerationStructure)),
		m_DescriptorPool(std::move(other.m_DescriptorPool)),
		m_DescriptorSetLayouts(std::move(other.m_DescriptorSetLayouts)),
		m_DescriptorSets(std::move(other.m_DescriptorSets))
	{
		other.m_Pipeline = nullptr;
		other.m_AccelerationStructure = nullptr;
		other.m_DescriptorPool = nullptr;
		other.m_DescriptorSetLayouts.clear();
		other.m_DescriptorSets.clear();
	}
	SceneRenderer& operator=(SceneRenderer&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_Pipeline = std::move(rhs.m_Pipeline);
			m_AccelerationStructure = std::move(rhs.m_AccelerationStructure);
			m_DescriptorPool = std::move(rhs.m_DescriptorPool);
			m_DescriptorSetLayouts = std::move(rhs.m_DescriptorSetLayouts);
			m_DescriptorSets = std::move(rhs.m_DescriptorSets);

			rhs.m_Pipeline = nullptr;
			rhs.m_AccelerationStructure = nullptr;
			rhs.m_DescriptorPool = nullptr;
			rhs.m_DescriptorSetLayouts.clear();
			rhs.m_DescriptorSets.clear();
		}

		return *this;
	}

public:
	struct RenderSceneInfo
	{
		uint32_t frameIndex;
		uint16_t width;
		uint16_t height;
		vk::ImageView outputImage;
		vk::CommandBuffer commandBuffer;

		RenderSceneInfo(
			uint32_t inFrameIndex,
			const uint16_t inWidth,
			const uint16_t inHeight,
			const vk::ImageView inOutputImage,
			const vk::CommandBuffer inCommandBuffer
		)
			: frameIndex(inFrameIndex),
			width(inWidth),
			height(inHeight),
			outputImage(inOutputImage),
			commandBuffer(inCommandBuffer)
		{}
	};
	void RenderScene(const RenderSceneInfo& inInfo);

protected:
	VulkanRaytracingPipeline m_Pipeline;
	VoxelAccelerationStructure m_AccelerationStructure;
	vk::DescriptorPool m_DescriptorPool;
	std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
	std::vector<vk::DescriptorSet> m_DescriptorSets;
};
