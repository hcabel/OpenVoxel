#pragma once

#include "VulkanRayTracingPipeline.h"

#include <vulkan/vulkan.hpp>

class SceneRenderer
{
public:

protected:
	SceneRenderer();
	~SceneRenderer();

protected:
	vk::DescriptorSetLayout m_DescriptorSetLayout;
	vk::DescriptorSet m_DescriptorSet;
	VulkanRayTracingPipeline m_Pipeline;

};
