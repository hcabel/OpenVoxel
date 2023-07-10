#pragma once

#include "Renderer_API.h"
#include "Vulkan/VulkanUtils.h"

#include <vulkan/vulkan.hpp>
#include <vector>

class RENDERER_API VulkanInstanceHandler final
{

public:
	VulkanInstanceHandler() = default;

	VulkanInstanceHandler(const VulkanInstanceHandler& rhs) = delete;
	VulkanInstanceHandler(VulkanInstanceHandler&& rhs) noexcept = delete;
	VulkanInstanceHandler& operator=(const VulkanInstanceHandler& rhs) = delete;
	VulkanInstanceHandler& operator=(VulkanInstanceHandler&& rhs) noexcept = delete;

	operator vk::Instance() const { return Raw(); }
	operator VkInstance() const { return RawC(); }

public:
	/**
	 * Create the vulkan instance.
	 *
	 * \param name the engine and application name
	 * \param fix the vulkan version to use (UINT32_MAX = latest)
	 * \param minor the vulkan version to use (UINT32_MAX = latest)
	 * \param major the vulkan version to use (UINT32_MAX = latest)
	 */
	void CreateInstance(const char* name, uint32_t fix = UINT32_MAX, uint32_t minor = UINT32_MAX, uint32_t major = UINT32_MAX);
	/** Destroy the vulkan instance */
	void DestroyInstance();

	/** Add an extension to the vulkan instance */
	void AddExtension(const char* extension);
	/** Add a layer to the vulkan instance */
	void AddLayer(const char* layer);

private:
	/** Tell whether or not all the vulkan instance extension in m_Extension are supported */
	bool allExtensionsAreSupported();
	/** Tell whether or not all the vulkan instance layers in m_Layers are supported */
	bool allLayersAreSupported();

public:
	/** Access the raw instance (Vulkan C++ API) */
	vk::Instance Raw() const { return (m_Instance); }
	/** Access the raw instance (Vulkan C API) */
	VkInstance RawC() const { return (m_Instance); }

private:
	// The real vulkan instance
	vk::Instance m_Instance;
	// The version of the vulkan instance
	uint32_t m_Version = VK_API_VERSION_1_3;
	// Whether or not the vulkan instance is created
	bool m_IsInstanceCreated = false;

	// The vulkan instance extensions that you wish to use
	std::vector<const char*> m_Extensions;
	// The vulkan instance layers that you wish to use
 	std::vector<const char*> m_Layers;
};
