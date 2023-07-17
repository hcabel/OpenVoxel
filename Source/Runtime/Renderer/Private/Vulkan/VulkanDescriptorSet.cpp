#include "Vulkan/VulkanDescriptorSet.h"
#include "Vulkan/VulkanDeviceHandler.h"
#include "Vulkan/VulkanAccelerationStructure.h"

#include <vector>

void VulkanDescriptorSet::CreateDescriptorSet(const vk::ImageView& raytracingImageView)
{
	std::vector<vk::DescriptorSetLayoutBinding> layoutBinding;

	// Which shader got access to the AS (raygen)
	layoutBinding.push_back(
		vk::DescriptorSetLayoutBinding(
			0,
			vk::DescriptorType::eAccelerationStructureKHR,
			1,
			vk::ShaderStageFlagBits::eRaygenKHR
		)
	);

	// Which shader got access to the image (raygen), meaning only raygen can write to the image
	layoutBinding.push_back(
		vk::DescriptorSetLayoutBinding(
			1,
			vk::DescriptorType::eStorageImage,
			1,
			vk::ShaderStageFlagBits::eRaygenKHR
		)
	);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.push_back(
		vk::DescriptorPoolSize(
			vk::DescriptorType::eAccelerationStructureKHR,
			1
		)
	);
	poolSizes.push_back(
		vk::DescriptorPoolSize(
			vk::DescriptorType::eStorageImage,
			1
		)
	);

	vk::DescriptorPoolCreateInfo poolInfo(
		vk::DescriptorPoolCreateFlags(),
		1,
		poolSizes.size(), poolSizes.data()
	);

	m_DescriptorPool = m_VkDevice->Raw().createDescriptorPool(poolInfo);

	vk::DescriptorSetLayoutCreateInfo layoutInfo(
		vk::DescriptorSetLayoutCreateFlags(),
		layoutBinding.size(), layoutBinding.data()
	);

	m_DescriptorSetLayout = m_VkDevice->Raw().createDescriptorSetLayout(layoutInfo);

	vk::DescriptorSetAllocateInfo allocInfo(
		m_DescriptorPool,
		1, &m_DescriptorSetLayout
	);

	m_DescriptorSet = m_VkDevice->Raw().allocateDescriptorSets(allocInfo)[0];

	vk::AccelerationStructureKHR accelerationStructure = m_VkAccelerationStructure->GetTlas();
	vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo(
		1, &accelerationStructure
	);

	vk::DescriptorImageInfo imageInfo(
		vk::Sampler(),
		raytracingImageView,
		vk::ImageLayout::eGeneral
	);

	std::vector<vk::WriteDescriptorSet> descriptorWrites;
	descriptorWrites.push_back(
		vk::WriteDescriptorSet(
			m_DescriptorSet,
			0,
			0,
			1, vk::DescriptorType::eAccelerationStructureKHR,
			nullptr,
			nullptr,
			nullptr,
			&accelerationStructureInfo
		)
	);
	descriptorWrites.push_back(
		vk::WriteDescriptorSet(
			m_DescriptorSet,
			1,
			0,
			1, vk::DescriptorType::eStorageImage,
			&imageInfo,
			nullptr,
			nullptr
		)
	);

	m_VkDevice->Raw().updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanDescriptorSet::DestroyDescriptorSet()
{
	m_VkDevice->Raw().destroyDescriptorSetLayout(m_DescriptorSetLayout);
	m_VkDevice->Raw().destroyDescriptorPool(m_DescriptorPool);
}
