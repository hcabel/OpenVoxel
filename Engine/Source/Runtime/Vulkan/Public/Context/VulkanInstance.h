#pragma once

#include "Vulkan_API.h"

#include <vulkan/vulkan.hpp>

class VULKAN_API VulkanInstance : public vk::Instance
{

protected:
	VulkanInstance& operator=(VulkanInstance* rhs)
	{
		vk::Instance::operator=(*rhs);
		m_LayerNames = std::move(rhs->m_LayerNames);
		m_ExtensionNames = std::move(rhs->m_ExtensionNames);
		m_Version = rhs->m_Version;
		return *this;
	}

protected:
	VulkanInstance()
		: vk::Instance(VK_NULL_HANDLE),
		m_LayerNames(),
		m_ExtensionNames(),
		m_Version(0)
	{}
	VulkanInstance(
		vk::Instance& instance,
		std::vector<const char*>& layerNames,
		std::vector<const char*>& extensionNames,
		uint32_t vulkanVersion
	)
		: vk::Instance(instance),
		m_LayerNames(std::move(layerNames)),
		m_ExtensionNames(std::move(extensionNames)),
		m_Version(vulkanVersion)
	{}

	/**
	 * Create the vulkan instance.
	 * @note The vulkan instance is set internally, so once you call this function you can use this class as a vk::Instance.
	 *
	 * @param vulkanApiVersion The vulkan version to use, if UINT32_MAX (default value), the latest version will be used.
	 * @return True if the instance was created successfully, false otherwise.
	 */
	bool Create(uint32_t vulkanApiVersion = UINT32_MAX);
	void Destroy();

public:
	__forceinline void AddLayer(const char* layerName) { m_LayerNames.push_back(layerName); }
	__forceinline void AddExtension(const char* extensionName) { m_ExtensionNames.push_back(extensionName); }

	/**
	 * Select the vulkan version to use, either the latest version or the one you specified.
	 *
	 * @param major The major version to use
	 * @param minor The minor version to use
	 * @param patch The patch version to use
	 * @return The vulkan version to use.
	 */
	static uint32_t SelectVulkanApiVersion(uint32_t major = UINT32_MAX, uint32_t minor = UINT32_MAX, uint32_t patch = UINT32_MAX);

	/**
	 * Check if the given layers are supported.
	 *
	 * @param layerNames The layers to check.
	 * @return True if all the layers are supported, false otherwise.
	 */
	static bool SupportAllInstanceLayer(const std::vector<const char*>& layerNames);

	/**
	 * Check if the given extensions are supported.
	 *
	 * @param extensionNames The extensions to check.
	 * @return True if all the extensions are supported, false otherwise.
	 */
	static bool SupportAllInstanceExtension(const std::vector<const char*>& extensionNames);

protected:
	std::vector<const char*> m_LayerNames;
	std::vector<const char*> m_ExtensionNames;
	/* The vulkan version being used */
	uint32_t m_Version;

	friend class VulkanContext;
};
