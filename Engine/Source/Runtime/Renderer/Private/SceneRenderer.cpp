#include "SceneRenderer.h"
#include "VulkanContext.h"

SceneRenderer::SceneRenderer()
{
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
	m_DescriptorSetLayout = VulkanContext::GetDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	// Create the descriptor set
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(
		VK_NULL_HANDLE,
		1,
		&m_DescriptorSetLayout
	);
	m_DescriptorSet = VulkanContext::GetDevice().allocateDescriptorSets(descriptorSetAllocateInfo)[0];
}

SceneRenderer::~SceneRenderer()
{

	VulkanContext::GetDevice().destroyDescriptorSetLayout(m_DescriptorSetLayout);
}
