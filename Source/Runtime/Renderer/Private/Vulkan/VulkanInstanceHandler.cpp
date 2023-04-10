#include "Vulkan/VulkanInstanceHandler.h"

void VulkanInstanceHandler::CreateInstance(const char* name, uint32_t fix, uint32_t minor, uint32_t major)
{
	vk::Result result;

	result = vk::enumerateInstanceVersion(&m_Version);
	CHECK_VULKAN_RESULT(result, "Unable to enumerate Instance Version")

	if (major != UINT32_MAX || minor != UINT32_MAX || fix != UINT32_MAX)
	{
		m_Version = VK_MAKE_VERSION(
			major == UINT32_MAX ? VK_API_VERSION_MAJOR(m_Version) : major,
			minor == UINT32_MAX ? VK_API_VERSION_MINOR(m_Version) : minor,
			fix == UINT32_MAX ? VK_API_VERSION_PATCH(m_Version) : fix
		);
	}

	vk::ApplicationInfo applicationInfo(
		name,
		m_Version,
		name,
		m_Version,
		m_Version
	);

	vk::InstanceCreateInfo instanceCreateInfo(
		vk::InstanceCreateFlags(),
		&applicationInfo,
		static_cast<uint32_t>(m_Layers.size()), m_Layers.data(),
		static_cast<uint32_t>(m_Extensions.size()), m_Extensions.data()
	);

	if (allExtensionsAreSupported() == false || allLayersAreSupported() == false)
	{
		OV_LOG(Fatal, LogVulkan, "Vulkan instance \"{:s}\" creation failed", name);
		return;
	}

	m_Instance = vk::createInstance(instanceCreateInfo);

	OV_LOG(Verbose, LogVulkan, "Vulkan instance \"{:s}\" created (version: {:d}.{:d}.{:d})",
		name, VK_API_VERSION_MAJOR(m_Version), VK_API_VERSION_MINOR(m_Version), VK_API_VERSION_PATCH(m_Version));
	OV_LOG(Verbose, LogVulkan, "Vulkan instance extension enabled:");
	OV_LOG_ARRAY(Verbose, LogVulkan, m_Extensions, "\t\"{:s}\"");
	OV_LOG(Verbose, LogVulkan, "Vulkan instance layers enabled:");
	OV_LOG_ARRAY(Verbose, LogVulkan, m_Layers, "\t\"{:s}\"");

	m_IsInstanceCreated = true;
}

void VulkanInstanceHandler::DestroyInstance()
{
	CHECKF(m_IsInstanceCreated, "Vulkan instance has never been created");

	m_Instance.destroy();

	m_Extensions.clear();
	m_Layers.clear();

	m_IsInstanceCreated = false;
}

void VulkanInstanceHandler::AddExtension(const char* extension)
{
	if (m_IsInstanceCreated == false)
	{
		m_Extensions.push_back(extension);
	}
}

void VulkanInstanceHandler::AddLayer(const char* layer)
{
	if (m_IsInstanceCreated == false)
	{
		m_Layers.push_back(layer);
	}
}

bool VulkanInstanceHandler::allExtensionsAreSupported()
{
	bool areTheyAllSupported = true;
	bool found;
	auto extensionProperties = vk::enumerateInstanceExtensionProperties();

	for (const char* extension : m_Extensions)
	{
		found = false;
		for (const vk::ExtensionProperties& extensionProperty : extensionProperties)
		{
			if (strcmp(extension, extensionProperty.extensionName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			OV_LOG_IF(areTheyAllSupported == true, Error, LogVulkan, "Unsupported instance extension list:")
			OV_LOG(Error, LogVulkan, "\t\"{:s}\"", extension);
			areTheyAllSupported = false;
		}
	}
	return (areTheyAllSupported);
}

bool VulkanInstanceHandler::allLayersAreSupported()
{
	bool areTheyAllSupported = true;
	bool found;
	auto layerProperties = vk::enumerateInstanceLayerProperties();

	for (const char* layer : m_Layers)
	{
		found = false;
		for (const vk::LayerProperties& layerProperty : layerProperties)
		{
			if (strcmp(layer, layerProperty.layerName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			OV_LOG_IF(areTheyAllSupported == true, Error, LogVulkan, "Unsupported instance layer list:")
			OV_LOG(Error, LogVulkan, "\t\"{:s}\"", layer);
			areTheyAllSupported = false;
		}
	}
	return (areTheyAllSupported);
}