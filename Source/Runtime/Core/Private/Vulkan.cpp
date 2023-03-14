#include "Vulkan.h"

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

	std::vector<const char*> extensions = GetDeviceExtensions();

	OV_LOG(VeryVerbose, LogVulkan, "Vulkan device extensions:");
	OV_LOG_ARRAY(VeryVerbose, LogVulkan, extensions, "\t\"{:s}\"");

	// select a GPU that can handle all our extensions and layers
	auto physicalDevice = GetPhysicalDevice(instance, layers, extensions);
	if (physicalDevice.has_value() == false)
		return {};
	device.Physical = physicalDevice.value();

#if WITH_LOGGING
	const auto deviceProperties = device.Physical.getProperties();
	OV_LOG(Verbose, LogVulkan, "Selected GPU: \"{:s}\"", deviceProperties.deviceName.data());// , vk::to_string(static_cast<vk::PhysicalDeviceType>(deviceProperties.deviceType)));
#endif

	// Create the logical device for this GPU
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device.Physical, surface);
	auto logicalDevice = CreateLogicalDevice(device.Physical, layers, extensions, queueFamilyIndices);
	if (logicalDevice.has_value() == false)
		return {};
	device.Logical = logicalDevice.value();

	device.Queues[QueueType::Graphics] = device.Logical.getQueue(queueFamilyIndices.Graphics, 0);
	device.Queues[QueueType::Present] = device.Logical.getQueue(queueFamilyIndices.Present, 0);

	OV_LOG(VeryVerbose, LogVulkan, "Queue created:");
	OV_LOG(VeryVerbose, LogVulkan, "\tGraphics: {:d}", queueFamilyIndices.Graphics);
	OV_LOG(VeryVerbose, LogVulkan, "\tPresent: {:d}", queueFamilyIndices.Present);

	return (device);
}

std::optional<Vulkan::SwapChainBundle> Vulkan::CreateSwapChain(const vk::Instance &instance, const DeviceBundle &device, const vk::SurfaceKHR &surface, uint32_t width, uint32_t height) noexcept
{
	SwapChainSupportDetails supportDetails = QuerySwapChainSupport(device, surface);

	vk::SurfaceFormatKHR selectedSurfaceFormat = SelectSwapChainSurfaceFormat(supportDetails.Formats);
	OV_LOG(Verbose, LogVulkan, "SwapChain format {:s} ({:s})", vk::to_string(selectedSurfaceFormat.format), vk::to_string(selectedSurfaceFormat.colorSpace));

	vk::PresentModeKHR selectedPresentMode = SelectSwapChainPresentMode(supportDetails.PresentModes);
	OV_LOG(Verbose, LogVulkan, "SwapChain present mode {:s}", vk::to_string(selectedPresentMode));

	vk::Extent2D selectedExtent = SelectSwapChainExtent(supportDetails.Capabilities, width, height);
	OV_LOG(Verbose, LogVulkan, "SwapChain extent {:d}x{:d}", selectedExtent.width, selectedExtent.height);

	uint32_t imageCount = std::min(
		supportDetails.Capabilities.minImageCount + 1,
		supportDetails.Capabilities.maxImageCount
	);

	vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
		vk::SwapchainCreateFlagsKHR(),
		surface,
		imageCount,
		selectedSurfaceFormat.format,
		selectedSurfaceFormat.colorSpace,
		selectedExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment
	);

	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device.Physical, surface);

	if (queueFamilyIndices.Graphics != queueFamilyIndices.Present)
	{
		uint32_t queueFamilyIndicesArray[] = { queueFamilyIndices.Graphics, queueFamilyIndices.Present };
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
	}
	else
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;

	createInfo.preTransform = supportDetails.Capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = selectedPresentMode;
	createInfo.clipped = VK_TRUE;

	SwapChainBundle swapChain;
	try {
		swapChain.SwapChain = device.Logical.createSwapchainKHR(createInfo);
	}
	catch (vk::SystemError &e)
	{
		OV_LOG(Error, LogVulkan, "Failed to create swap chain: \"{:s}\"", e.what());
		return {};
	}

	std::vector<vk::Image> images = device.Logical.getSwapchainImagesKHR(swapChain.SwapChain);
	swapChain.Frames.resize(images.size());
	for (size_t i = 0; i < images.size(); i++)
		swapChain.Frames[i] = CreateSwapChainFrame(device, images[i], selectedSurfaceFormat.format);

	swapChain.SurfaceFormat = selectedSurfaceFormat.format;
	swapChain.Extent = selectedExtent;
	return (swapChain);
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

std::vector<const char*> Vulkan::GetDeviceExtensions() noexcept
{
	std::vector<const char*> extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	return (extensions);
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
			return (false);
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
			return (false);
	}
	return (true);
}

std::optional<vk::Device> Vulkan::CreateLogicalDevice(const vk::PhysicalDevice& device, const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const QueueFamilyIndices &familyIndices) noexcept
{
	std::vector<vk::DeviceQueueCreateInfo> queues = { CreateQueue(device, familyIndices.Graphics) };

	// Present and Graphics family is most likely handle by the same queue, but just in case is doesn't we added a new queue for Presenting
	if (familyIndices.Present != familyIndices.Graphics)
		queues.push_back(CreateQueue(device, familyIndices.Present));

	vk::PhysicalDeviceFeatures features = vk::PhysicalDeviceFeatures();
	vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo(
		vk::DeviceCreateFlags(),
		static_cast<uint32_t>(queues.size()), queues.data(),
		static_cast<uint32_t>(layers.size()), layers.data(),
		static_cast<uint32_t>(extensions.size()), extensions.data(),
		&features
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

# pragma region SwapChain
Vulkan::SwapChainSupportDetails Vulkan::QuerySwapChainSupport(const DeviceBundle& device, const vk::SurfaceKHR& surface) noexcept
{
	SwapChainSupportDetails supportDetails;

	supportDetails.Capabilities = device.Physical.getSurfaceCapabilitiesKHR(surface);
	supportDetails.Formats = device.Physical.getSurfaceFormatsKHR(surface);
	supportDetails.PresentModes = device.Physical.getSurfacePresentModesKHR(surface);

	return (supportDetails);
}

vk::SurfaceFormatKHR Vulkan::SelectSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) noexcept
{
	for (auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb
			&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return (availableFormat);
	}
	return (availableFormats[0]);
}

vk::PresentModeKHR Vulkan::SelectSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) noexcept
{
	for (auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			return (availablePresentMode);
	}
	return (vk::PresentModeKHR::eFifo); // FIFO is guaranteed to be available
}

vk::Extent2D Vulkan::SelectSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) noexcept
{
	if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{
		vk::Extent2D actualExtent = vk::Extent2D(width, height);

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return (actualExtent);
	}
	else
		return (capabilities.currentExtent);
}

Vulkan::SwapChainFrame Vulkan::CreateSwapChainFrame(const DeviceBundle& device, const vk::Image& image, const vk::Format& imageFormat) noexcept
{
	SwapChainFrame frame;

	frame.Image = image;

	vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		imageFormat,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
	);

	frame.View = device.Logical.createImageView(createInfo);
	return (frame);
}
# pragma endregion
#pragma endregion