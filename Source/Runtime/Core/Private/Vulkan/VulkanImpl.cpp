#include "Vulkan/VulkanImpl.h"

DEFINE_LOG_CATEGORY(LogVulkan);
#if OV_DEBUG
DEFINE_LOG_CATEGORY(LogVulkanInternal);
#endif

std::optional<vk::Instance> Vulkan::CreateInstance(const std::vector<const char*>& additionalExtensions, const std::vector<const char*>& additionalLayers) noexcept
{
	/* Version */
	uint32_t vulkanVersion = GetVulkanVersion();
	if (vulkanVersion == 0)
		return {};

	/* Application Info */
	vk::ApplicationInfo applicationInfo = vk::ApplicationInfo(
		"OpenVoxel",
		vulkanVersion,
		"OpenVoxel",
		vulkanVersion,
		vulkanVersion
	);

	/* Layers */
	std::vector<const char*> layers = GetRequiredInstanceLayers();
	layers.insert(layers.end(), additionalLayers.begin(), additionalLayers.end());

	OV_LOG(VeryVerbose, LogVulkan, "Vulkan instance layers:");
	OV_LOG_ARRAY(VeryVerbose, LogVulkan, layers, "\t\"{:s}\"");

	if (IsLayersSupported(layers) == false)
		return {};

	/* Extensions */
	std::vector<const char*> extensions = GetRequiredInstanceExtensions();
	extensions.insert(extensions.end(), additionalExtensions.begin(), additionalExtensions.end());

	OV_LOG(VeryVerbose, LogVulkan, "Vulkan instance extensions:");
	OV_LOG_ARRAY(VeryVerbose, LogVulkan, extensions, "\t\"{:s}\"");

	if (IsExtensionsSupported(extensions) == false)
		return {};

	/* Create instance */
	vk::InstanceCreateInfo createInfo(
		vk::InstanceCreateFlags(),
		&applicationInfo,
		static_cast<uint32_t>(layers.size()), layers.data(),
		static_cast<uint32_t>(extensions.size()), extensions.data()
	);

	try {
		return (vk::createInstance(createInfo));
	}
	catch (vk::SystemError& e)
	{
		OV_LOG(Fatal, LogVulkan, "Failed to create vulkan instance: \"{:s}\"", e.what());
		return {};
	}
}

#if OV_DEBUG
VkBool32 VulkanDebugLogInternal(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		OV_LOG(VeryVerbose, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		OV_LOG(VeryVerbose, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		OV_LOG(Warning, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		OV_LOG(Error, LogVulkanInternal, "{:s}", pCallbackData->pMessage);
		break;
	}
	return (VK_FALSE);
}

Vulkan::DebugMessenger Vulkan::SetupDebugMessenger(const vk::Instance &instance) noexcept
{
	DebugMessenger debugMessenger;
	debugMessenger.Dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	vk::DebugUtilsMessengerCreateInfoEXT createInfo(
		vk::DebugUtilsMessengerCreateFlagsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
		VulkanDebugLogInternal
	);

	debugMessenger.Messenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, debugMessenger.Dldi);
	return (debugMessenger);
}
#endif // OV_DEBUG

std::optional<vk::SurfaceKHR> Vulkan::CreateSurface(const vk::Instance instance, GLFWwindow* window) noexcept
{
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		return {};
	return (vk::SurfaceKHR(surface));
}

std::optional<Vulkan::DeviceBundle> Vulkan::CreateDevice(const vk::Instance &instance, const vk::SurfaceKHR &surface) noexcept
{
	DeviceBundle device;

	std::vector<const char*> layers = GetDeviceLayers();

	OV_LOG(VeryVerbose, LogVulkan, "Vulkan device layers:");
	OV_LOG_ARRAY(VeryVerbose, LogVulkan, layers, "\t\"{:s}\"");

	auto deviceExtensions = GetDeviceExtensions();

	OV_LOG(VeryVerbose, LogVulkan, "Vulkan device extensions:");
	OV_LOG_ARRAY(VeryVerbose, LogVulkan, deviceExtensions.GetExtensions(), "\t\"{:s}\"");

	// select a GPU that can handle all our extensions and layers
	auto physicalDevice = GetPhysicalDevice(instance, layers, deviceExtensions.GetExtensions());
	if (physicalDevice.has_value() == false)
		return {};
	device.Physical = physicalDevice.value();

#if WITH_LOGGING
	const auto deviceProperties = device.Physical.getProperties();
	OV_LOG(Verbose, LogVulkan, "Selected GPU: \"{:s}\"", deviceProperties.deviceName.data());// , vk::to_string(static_cast<vk::PhysicalDeviceType>(deviceProperties.deviceType)));
#endif

	// Create the logical device for this GPU
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device.Physical, surface);
	auto logicalDevice = CreateLogicalDevice(device.Physical, layers, deviceExtensions, queueFamilyIndices);
	if (logicalDevice.has_value() == false)
		return {};
	device.Logical = logicalDevice.value();

	device.Queues[QueueType::Graphics].Queue = device.Logical.getQueue(queueFamilyIndices.Graphics, 0);
	device.Queues[QueueType::Graphics].FamilyIndex = queueFamilyIndices.Graphics;
	device.Queues[QueueType::Present].Queue = device.Logical.getQueue(queueFamilyIndices.Present, 0);
	device.Queues[QueueType::Present].FamilyIndex = queueFamilyIndices.Present;

	OV_LOG(VeryVerbose, LogVulkan, "Queue created:");
	OV_LOG(VeryVerbose, LogVulkan, "\tGraphics: {:d}", queueFamilyIndices.Graphics);
	OV_LOG(VeryVerbose, LogVulkan, "\tPresent: {:d}", queueFamilyIndices.Present);

	return (device);
}

#pragma region Private
# pragma region VkInstance
uint32_t Vulkan::GetVulkanVersion()
{
	uint32_t vulkanVersion;
	vk::Result result = vk::enumerateInstanceVersion(&vulkanVersion);
	if (result != vk::Result::eSuccess)
	{
		OV_LOG(Error, LogVulkan, "Failed to get vulkan version: \"{:s}\"", vk::to_string(result));
		return (0);
	}

	// Remove patch version for more compatibility/stability
	vulkanVersion = VK_MAKE_VERSION(VK_VERSION_MAJOR(vulkanVersion), VK_VERSION_MINOR(vulkanVersion), 0);
	OV_LOG(Verbose, LogVulkan, "Vulkan version: {:d}.{:d}.{:d}", VK_VERSION_MAJOR(vulkanVersion), VK_VERSION_MINOR(vulkanVersion), VK_VERSION_PATCH(vulkanVersion));

	return (vulkanVersion);
}

std::vector<const char*> Vulkan::GetRequiredInstanceExtensions()
{
	uint32_t count = 0;
	const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&count);

	std::vector<const char*> extensions = std::vector<const char*>(glfwRequiredExtensions, glfwRequiredExtensions + count);

#if OV_DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Log internal vulkan messages
#endif

	return (extensions);
}

bool Vulkan::IsExtensionsSupported(const std::vector<const char*>& extensions)
{
	auto extensionProperties = vk::enumerateInstanceExtensionProperties();

	for (const char* extension : extensions)
	{
		bool found = false;
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
			OV_LOG(Error, LogVulkan, "Extension \"{:s}\" is not supported.", extension);
			return (false);
		}
	}
	return (true);
}

std::vector<const char*> Vulkan::GetRequiredInstanceLayers()
{
	std::vector<const char*> layers;

#if OV_DEBUG
	layers.push_back("VK_LAYER_KHRONOS_validation"); // Enable "Debug Utils" extension
#endif

	return (layers);
}

bool Vulkan::IsLayersSupported(const std::vector<const char*>& layers)
{
	auto layerProperties = vk::enumerateInstanceLayerProperties();

	for (const char* layer : layers)
	{
		bool found = false;
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
			OV_LOG(Fatal, LogVulkan, "Layer \"{:s}\" is not supported.", layer);
			return (false);
		}
	}
	return (true);
}
# pragma endregion

# pragma region Device
std::optional<vk::PhysicalDevice> Vulkan::GetPhysicalDevice(const vk::Instance &instance, const std::vector<const char*>& layers, const std::vector<const char*>& extensions) noexcept
{

	auto devices = instance.enumeratePhysicalDevices();
	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device, layers, extensions))
			return (device);
	}
	return {};
}

std::vector<const char*> Vulkan::GetDeviceLayers() noexcept
{
	std::vector<const char*> layers;

#if OV_DEBUG
	layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	return (layers);
}

Vulkan::DeviceExtensionChain Vulkan::GetDeviceExtensions() noexcept
{
	DeviceExtensionChain chain;

	chain.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Allow to present rendering

	// Ray tracing pipeline
	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = {};
	rtPipelineFeatures.rayTracingPipeline = VK_TRUE;
	chain.AddExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rtPipelineFeatures);

	// Build acceleration structure
	vk::PhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = {};
	asFeatures.accelerationStructure = VK_TRUE;
	chain.AddExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &asFeatures);

	// Required by acceleration structure
	chain.AddExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	return (chain);
}

bool Vulkan::IsDeviceSuitable(const vk::PhysicalDevice& device, const std::vector<const char*>& layers, const std::vector<const char*>& extensions) noexcept
{
	/* Check Layers */
	auto layersProperties = device.enumerateDeviceLayerProperties();

	for (const char* layer : layers)
	{
		bool found = false;
		for (const vk::LayerProperties& layerProperty : layersProperties)
		{
			if (strcmp(layer, layerProperty.layerName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			OV_LOG(VeryVerbose, LogVulkan, "Layer \"{:s}\" is not supported by \"{:s}\"", layer, device.getProperties().deviceName.data());
			return (false);
		}
	}

	/* Check Extensions */
	auto extensionsProperties = device.enumerateDeviceExtensionProperties();

	for (const char* extension : extensions)
	{
		bool found = false;
		for (const vk::ExtensionProperties& extensionProperty : extensionsProperties)
		{
			if (strcmp(extension, extensionProperty.extensionName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			OV_LOG(VeryVerbose, LogVulkan, "Extension \"{:s}\" is not supported by \"{:s}\"", extension, device.getProperties().deviceName.data());
			return (false);
		}
	}
	return (true);
}

std::optional<vk::Device> Vulkan::CreateLogicalDevice(const vk::PhysicalDevice& device, const std::vector<const char*>& layers, const DeviceExtensionChain& extensions, const QueueFamilyIndices &familyIndices) noexcept
{
	std::vector<vk::DeviceQueueCreateInfo> queues = { CreateQueue(device, familyIndices.Graphics) };

	// Present and Graphics family is most likely handle by the same queue, but just in case is doesn't we added a new queue for Presenting
	if (familyIndices.Present != familyIndices.Graphics)
		queues.push_back(CreateQueue(device, familyIndices.Present));

	const std::vector<const char*> extensionsNames = extensions.GetExtensions();
	void* pNextChain = extensions.ConstructPNextChain();

	vk::PhysicalDeviceFeatures features = {};
	vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo(
		vk::DeviceCreateFlags(),
		static_cast<uint32_t>(queues.size()), queues.data(),
		static_cast<uint32_t>(layers.size()), layers.data(),
		static_cast<uint32_t>(extensionsNames.size()), extensionsNames.data(),
		&features
	//	pNextChain
	);

	try {
		return (device.createDevice(createInfo));
	}
	catch (const vk::SystemError& e)
	{
		OV_LOG(Error, LogVulkan, "Failed to create logical device: {:s}", e.what());
		return {};
	}
}
# pragma endregion

# pragma region Queue
Vulkan::QueueFamilyIndices Vulkan::FindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) noexcept
{
	QueueFamilyIndices indices;

	auto queueFamilies = device.getQueueFamilyProperties();
	uint32_t i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			indices.Graphics = i;

		if (device.getSurfaceSupportKHR(i, surface))
			indices.Present = i;

		if (indices.IsComplete())
			break;
		i++;
	}
	return (indices);
}

vk::DeviceQueueCreateInfo Vulkan::CreateQueue(const vk::PhysicalDevice& device, uint32_t queueFamilyIndex) noexcept
{
	float queuePriority = 1.0f;
	vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo(
		vk::DeviceQueueCreateFlags(),
		queueFamilyIndex,
		1,
		&queuePriority
	);
	return (queueCreateInfo);
}
# pragma endregion
#pragma endregion