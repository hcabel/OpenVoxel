#include "VulkanInstance.h"
#include "Version.h"
#include "Vulkan/ErrorHandling.h"
#include "Vulkan/Log.h"

bool VulkanInstance::Create(uint32_t vulkanApiVersion)
{
	if (vulkanApiVersion != UINT32_MAX)
		m_Version = vulkanApiVersion;
	else
		m_Version = SelectVulkanApiVersion(); // use the latest available

	VULKAN_LOG(Verbose, "Using Vulkan version: {:d}.{:d}.{:d}",
		VK_API_VERSION_MAJOR(m_Version),
		VK_API_VERSION_MINOR(m_Version),
		VK_API_VERSION_PATCH(m_Version)
	);

#ifdef OV_DEBUG
	// In debug mode we add the validation layer, it will provide us with some useful information when something goes wrong
	AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	AddLayer("VK_LAYER_KHRONOS_validation");
#endif

	if (SupportAllInstanceLayer(m_LayerNames) == false)
	{
		VULKAN_LOG(Error, "Not all instance layers are supported!");
		return (false);
	}
	if (SupportAllInstanceExtension(m_ExtensionNames) == false)
	{
		VULKAN_LOG(Error, "Not all instance extensions are supported!");
		return (false);
	}

	VULKAN_LOG(Verbose, "Instance layers:");
	VULKAN_LOG_ARRAY(m_LayerNames, Verbose, "\t\"{:s}\"");
	VULKAN_LOG(Verbose, "Instance extensions:");
	VULKAN_LOG_ARRAY(m_ExtensionNames, Verbose, "\t\"{:s}\"");

	vk::ApplicationInfo applicationInfo(
		// TODO: replace the next to value by the values of the project
		"[Project Name]",
		VK_MAKE_API_VERSION(42, 42, 42, 42), // I hope that if I forget people will catch up that this is unusual ^^
		"Open Voxel",
		OV_CURRENT_VERSION,
		vulkanApiVersion
	);

	vk::InstanceCreateInfo createInfo(
		vk::InstanceCreateFlags(),
		&applicationInfo,
		static_cast<uint32_t>(m_LayerNames.size()), m_LayerNames.data(),
		static_cast<uint32_t>(m_ExtensionNames.size()), m_ExtensionNames.data()
	);

	try
	{
		vk::Instance instance = vk::createInstance(createInfo);
		*this = VulkanInstance(instance, m_LayerNames, m_ExtensionNames, m_Version);
		return (true);
	}
	catch (vk::SystemError& e)
	{
		VULKAN_LOG(Error, "Failed to create instance: \"{:s}\"", e.what());
		return (false);
	}
}

void VulkanInstance::Destroy()
{
	if (static_cast<VkInstance>(*this) != VK_NULL_HANDLE)
	{
		VULKAN_LOG(Verbose, "Destroying Vulkan instance...");
		destroy();

		*this = VulkanInstance(); // reset the instance
	}
}

uint32_t VulkanInstance::SelectVulkanApiVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
	uint32_t latestVersionAvailable;

	CHECK_VK_RESULT(
		vk::enumerateInstanceVersion(&latestVersionAvailable),
		"Enumerating instance versions", Fatal
	);

	// If you haven't specified a version, we use the latest available
	if (major == UINT32_MAX && minor == UINT32_MAX && patch == UINT32_MAX)
		return (latestVersionAvailable);

	// By default we resolve non valid values to 0
	major = (major > VK_VERSION_MAJOR(latestVersionAvailable) ? 0 : major);
	minor = (minor > VK_VERSION_MINOR(latestVersionAvailable) ? 0 : minor);
	patch = (patch > VK_VERSION_PATCH(latestVersionAvailable) ? 0 : patch);

	// Create the version based on the individual values
	return (VK_MAKE_VERSION(major, minor, patch));
}

bool VulkanInstance::SupportAllInstanceLayer(const std::vector<const char *> &layerNames)
{
	auto layerProperties = vk::enumerateInstanceLayerProperties();

#ifdef WITH_LOGGING
	bool areAllLayerSupported = true;

	for (const char* layerName : layerNames)
	{
		bool layerFound = false;

		for (const auto& layerProperty : layerProperties)
		{
			if (strcmp(layerName, layerProperty.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			VULKAN_LOG(Error, "Layer \"{:s}\" is not supported!", layerName);
			areAllLayerSupported = false;
		}
	}

	return (areAllLayerSupported);
#else
	for (const char* layerName : layerNames)
	{
		bool layerFound = false;

		for (const auto& layerProperty : layerProperties)
		{
			if (strcmp(layerName, layerProperty.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return (false);
	}
#endif // WITH_LOGGING
}

bool VulkanInstance::SupportAllInstanceExtension(const std::vector<const char *> &extensionNames)
{
	auto extensionProperties = vk::enumerateInstanceExtensionProperties();

#ifdef WITH_LOGGING
	bool areAllExtensionSupported = true;

	for (const char* extensionName : extensionNames)
	{
		bool extensionFound = false;

		for (const auto& extensionProperty : extensionProperties)
		{
			if (strcmp(extensionName, extensionProperty.extensionName) == 0)
			{
				extensionFound = true;
				break;
			}
		}

		if (!extensionFound)
		{
			VULKAN_LOG(Error, "Extension \"{:s}\" is not supported!", extensionName);
			areAllExtensionSupported = false;
		}
	}

	return (areAllExtensionSupported);
#else
	for (const char* extensionName : extensionNames)
	{
		bool extensionFound = false;

		for (const auto& extensionProperty : extensionProperties)
		{
			if (strcmp(extensionName, extensionProperty.extensionName) == 0)
			{
				extensionFound = true;
				break;
			}
		}

		if (!extensionFound)
			return (false);
	}
#endif // WITH_LOGGING
}
