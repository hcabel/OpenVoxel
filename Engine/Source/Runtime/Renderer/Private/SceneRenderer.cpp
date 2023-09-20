#include "SceneRenderer.h"
#include "VulkanContext.h"
#include "Profiling/ProfilingMacros.h"

#include <glm/glm.hpp>

SceneRenderer SceneRenderer::Create(uint32_t frameCount)
{
	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.push_back(
		vk::DescriptorPoolSize(
			vk::DescriptorType::eAccelerationStructureKHR,
			1
		)
	);
	// Create the descriptor pool
	vk::DescriptorPool descriptorPool = VulkanContext::GetDevice().createDescriptorPool(
		vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			frameCount,
			poolSizes.size(), poolSizes.data()
		)
	);

	// Describe what is in the descriptor set and which shader has access to it
	std::array<vk::DescriptorSetLayoutBinding, 2> descriptorSetLayoutBindings;
	descriptorSetLayoutBindings[0] = vk::DescriptorSetLayoutBinding( // binding 0: Ray tracing image output
		0,
		vk::DescriptorType::eStorageImage,
		1,
		vk::ShaderStageFlagBits::eRaygenKHR
	);
	descriptorSetLayoutBindings[1] = vk::DescriptorSetLayoutBinding( // binding 1: The ray tracing acceleration structure
		1,
		vk::DescriptorType::eAccelerationStructureKHR,
		1,
		vk::ShaderStageFlagBits::eRaygenKHR
	);

	// Create the descriptor set layout
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
		vk::DescriptorSetLayoutCreateFlags(),
		static_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data()
	);

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(frameCount);
	for (uint32_t i = 0; i < frameCount; i++)
		descriptorSetLayouts[i] = VulkanContext::GetDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	// Create the descriptor set
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(
		descriptorPool,
		frameCount, descriptorSetLayouts.data(),
		nullptr
	);

	std::vector<vk::DescriptorSet> descriptorSets
		= VulkanContext::GetDevice().allocateDescriptorSets(descriptorSetAllocateInfo, VulkanContext::GetDispatcher());

	VoxelAccelerationStructure accelerationStructure = VoxelAccelerationStructure::Create();
	accelerationStructure.AddVoxel();
	accelerationStructure.Rebuild();

	// Create the pipeline
	VulkanRaytracingPipeline raytracingPipeline = VulkanRaytracingPipeline::Create(descriptorSetLayouts[0]);

	return (SceneRenderer(
		std::move(raytracingPipeline),
		std::move(accelerationStructure),
		std::move(descriptorPool),
		std::move(descriptorSetLayouts),
		std::move(descriptorSets)
	));
}

SceneRenderer::~SceneRenderer()
{
	if (m_DescriptorSets.size() > 0)
		VulkanContext::GetDevice().freeDescriptorSets(m_DescriptorPool, m_DescriptorSets.size(), m_DescriptorSets.data());

	if (m_DescriptorPool)
		VulkanContext::GetDevice().destroyDescriptorPool(m_DescriptorPool);

	for (uint32_t i = 0; i < m_DescriptorSetLayouts.size(); ++i)
		VulkanContext::GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayouts[i]);
}

void SceneRenderer::RenderScene(const SceneRenderer::RenderSceneInfo& inInfo)
{
	CREATE_PERFRAME_SCOPE_TIMER("Updating acceleration structure");

	if (m_AccelerationStructure.IsRebuildNeeded() == true)
		m_AccelerationStructure.Rebuild();

	inInfo.commandBuffer.bindPipeline(
		vk::PipelineBindPoint::eRayTracingKHR,
		m_Pipeline
	);

	// update descriptor set
	{
		std::vector<vk::WriteDescriptorSet> descriptorWrites;

		// Write the current image view on the descriptor set (layout location 1)
		vk::DescriptorImageInfo imageInfo(
			vk::Sampler(),
			inInfo.outputImage,
			vk::ImageLayout::eGeneral
		);
		descriptorWrites.push_back(
			vk::WriteDescriptorSet(
				m_DescriptorSets[inInfo.frameIndex],
				0,
				0,
				1, vk::DescriptorType::eStorageImage,
				&imageInfo,
				nullptr,
				nullptr
			)
		);

		// Get TLAS and write it on the descriptor set (layout location 0)
		vk::AccelerationStructureKHR accelerationStructure = m_AccelerationStructure.GetTLAS();
		vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo(
			1, &accelerationStructure
		);
		descriptorWrites.push_back(
			vk::WriteDescriptorSet(
				m_DescriptorSets[inInfo.frameIndex],
				1,
				0,
				1, vk::DescriptorType::eAccelerationStructureKHR,
				nullptr,
				nullptr,
				nullptr,
				&accelerationStructureInfo
			)
		);

		// Push all the update to the descriptor set (so it's actually updated)
		VulkanContext::GetDevice().updateDescriptorSets(
			descriptorWrites.size(), descriptorWrites.data(),
			0,
			nullptr
		);

		// Bind the descriptor set (allow shader to access the data stored in the descriptor set)
		inInfo.commandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR,
			m_Pipeline.GetLayout(),
			0,
			1, &m_DescriptorSets[inInfo.frameIndex],
			0,
			nullptr
		);
	}

	// Trace rays
	auto rgen = m_Pipeline.GetRaygenSbt();
	auto rmiss = m_Pipeline.GetMissSbt();
	auto rchit = m_Pipeline.GetHitSbt();
	auto rcall = m_Pipeline.GetCallableSbt();
	inInfo.commandBuffer.traceRaysKHR(
		rgen, rmiss, rchit, rcall,
		inInfo.width,
		inInfo.height,
		1,
		VulkanContext::GetDispatcher()
	);
}
