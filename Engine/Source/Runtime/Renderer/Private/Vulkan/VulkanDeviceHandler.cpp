#include "Vulkan/VulkanDeviceHandler.h"
#include "Vulkan/VulkanInstanceHandler.h"

#include <set>

VulkanDeviceHandler::VulkanDeviceHandler(const VulkanInstanceHandler* instance)
	: m_VkInstance(instance)
{}

void VulkanDeviceHandler::CreateDevice(const vk::SurfaceKHR& surface)
{
	CHECK(m_VkInstance);

	if (!m_PhysicalDevice)
		m_PhysicalDevice = FetchSuitablePhysicalDevice();

#if WITH_LOGGING
	// Print device infos:
	const auto deviceProperties = m_PhysicalDevice.getProperties();
	OV_LOG(LogVulkan, Verbose, "Selected GPU: \"{:s}\" ({:s})",
		deviceProperties.deviceName.data(), vk::to_string(deviceProperties.deviceType));
#endif

	if (allExtensionsAreSupported() == false || allLayersAreSupported() == false)
	{
		OV_LOG(LogVulkan, Fatal, "Unable to create the device");
		return;
	}

	OV_LOG(LogVulkan, Verbose, "Vulkan device extension enabled:");
	OV_LOG_ARRAY(LogVulkan, Verbose, m_Extensions, "\t\"{:s}\"");
	OV_LOG(LogVulkan, Verbose, "Vulkan device layers enabled:");
	OV_LOG_ARRAY(LogVulkan, Verbose, m_Layers, "\t\"{:s}\"");

	auto famillyIndices = FindQueueFamilyIndices(surface);
	auto queuesCreateInfo = SetupQueuesCreateInfo(famillyIndices);

	vk::PhysicalDeviceFeatures physicalDeviceFeatures = {};
	vk::DeviceCreateInfo deviceCreateInfo(
		vk::DeviceCreateFlags(),
		static_cast<uint32_t>(queuesCreateInfo.size()), queuesCreateInfo.data(),
		static_cast<uint32_t>(m_Layers.size()), m_Layers.data(),
		static_cast<uint32_t>(m_Extensions.size()), m_Extensions.data(),
		&physicalDeviceFeatures,
		m_FeatureChain.Begin()
	);
	m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);

	// Initialize m_Queues array with all the queue that has been create at the logical device creation
	for (uint8_t i = 0; i < VulkanQueueType::COUNT; i++)
	{
		VulkanQueueType::Type queueType = static_cast<VulkanQueueType::Type>(i);
		m_Queues[queueType] = VulkanQueue(m_Device.getQueue(famillyIndices[queueType], 0));
		m_Queues[queueType].FamilyIndex = famillyIndices[queueType];
	}

	m_PhysicalDeviceProperties = m_PhysicalDevice.getProperties();

	m_PhysicalDeviceProperties2.pNext = &m_RaytracingProperties;
	m_PhysicalDevice.getProperties2(&m_PhysicalDeviceProperties2);

	m_IsDeviceCreated = true;
}

void VulkanDeviceHandler::DestroyDevice()
{
	CHECK(m_IsDeviceCreated);

	m_Device.destroy();

	m_VkInstance = nullptr;

	m_IsDeviceCreated = false;
}

void VulkanDeviceHandler::AddExtension(const char* extension, void* feature)
{
	if (m_IsDeviceCreated == false)
	{
		m_Extensions.push_back(extension);
		if (feature)
			m_FeatureChain.PushBack(feature);
	}
}

void VulkanDeviceHandler::AddLayer(const char* layer)
{
	if (m_IsDeviceCreated == false)
	{
		m_Layers.push_back(layer);
	}
}

vk::PhysicalDevice VulkanDeviceHandler::FetchSuitablePhysicalDevice() const
{
	auto devices = m_VkInstance->Raw().enumeratePhysicalDevices();
	if (devices.empty())
		OV_LOG(LogVulkan, Fatal, "No GPU found");
	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
			return (device);
	}

	// If no suitable device was found, return the first one
	return (devices[0]);
}

bool VulkanDeviceHandler::IsDeviceSuitable(vk::PhysicalDevice physicalDevice) const
{
	/* Check if the physical device support the required extensions */
	auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
	for (const auto& extension : m_Extensions)
	{
		bool found = false;
		for (const auto& deviceExtension : extensions)
		{
			if (strcmp(extension, deviceExtension.extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			return (false);
	}

	/* Check if the physical device support the required layers */
	auto layers = physicalDevice.enumerateDeviceLayerProperties();
	for (const auto& layer : m_Layers)
	{
		bool found = false;
		for (const auto& deviceLayer : layers)
		{
			if (strcmp(layer, deviceLayer.layerName) == 0)
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

bool VulkanDeviceHandler::allExtensionsAreSupported() const
{
	bool areTheyAllSupported = true;
	bool found;
	auto extensionProperties = m_PhysicalDevice.enumerateDeviceExtensionProperties();

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
			OV_LOG_IF(areTheyAllSupported == true, LogVulkan, Error, "Unsupported device extension list:")
			OV_LOG(LogVulkan, Error, "\t\"{:s}\"", extension);
			areTheyAllSupported = false;
		}
	}
	return (areTheyAllSupported);
}

bool VulkanDeviceHandler::allLayersAreSupported() const
{
	bool areTheyAllSupported = true;
	bool found;
	auto layerProperties = m_PhysicalDevice.enumerateDeviceLayerProperties();

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
			OV_LOG_IF(areTheyAllSupported == true, LogVulkan, Error, "Unsupported device layer list:")
			OV_LOG(LogVulkan, Error, "\t\"{:s}\"", layer);
			areTheyAllSupported = false;
		}
	}
	return (areTheyAllSupported);
}

std::array<uint32_t, VulkanQueueType::COUNT> VulkanDeviceHandler::FindQueueFamilyIndices(const vk::SurfaceKHR& surface) const
{
	CHECK(m_PhysicalDevice, "No physical device selected, Unable to find queues indices's");
	std::array<uint32_t, VulkanQueueType::COUNT> queueFamilyIndices;
	// Fill with default value otherwise it will be filled with garbage or 0 (which is a valid index)
	queueFamilyIndices.fill(UINT32_MAX);

	auto queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyIndices[VulkanQueueType::Graphic] == UINT32_MAX
			&& queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			queueFamilyIndices[VulkanQueueType::Graphic] = i;
		if (queueFamilyIndices[VulkanQueueType::Present] == UINT32_MAX
			&& m_PhysicalDevice.getSurfaceSupportKHR(i, surface))
			queueFamilyIndices[VulkanQueueType::Present] = i;
		/* Not using compute nor transfer yet
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
			queueFamilyIndices[VulkanQueueType::Compute] = i;
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer)
			queueFamilyIndices[VulkanQueueType::Transfer] = i;
		*/
	}

	return (queueFamilyIndices);
}

std::vector<vk::DeviceQueueCreateInfo> VulkanDeviceHandler::SetupQueuesCreateInfo(const std::array<uint32_t, VulkanQueueType::COUNT>& queueFamilyIndices) const
{
	std::vector<vk::DeviceQueueCreateInfo> queuesCreateInfo;
	std::set<uint32_t> uniqueQueueFamillyIndices = { // a set will also remove duplicates
		queueFamilyIndices[VulkanQueueType::Graphic],
		queueFamilyIndices[VulkanQueueType::Present],
		// queueFamilyIndices[VulkanQueueType::Compute], // Not using compute nor transfer yet
		// queueFamilyIndices[VulkanQueueType::Transfer]
	};

	OV_LOG(LogVulkan, VeryVerbose, "Vulkan Queue families indices's:");

	float queuePriority = 1.0f;
	for (uint32_t uniqueQueueFamillyIndex : uniqueQueueFamillyIndices)
	{
		// UINT32_MAX is a value that I setup to tell that no family index is available
		if (uniqueQueueFamillyIndex != UINT32_MAX)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo(
				vk::DeviceQueueCreateFlags(),
				uniqueQueueFamillyIndex,
				1,
				&queuePriority
			);
			queuesCreateInfo.push_back(queueCreateInfo);

			OV_LOG(LogVulkan, VeryVerbose, "\tQueue family {:d} will be used for: {:s} ",
				uniqueQueueFamillyIndex,
				VulkanQueueType::ToString(
					uniqueQueueFamillyIndex == queueFamilyIndices[VulkanQueueType::Graphic] ? VulkanQueueType::Graphic :
					uniqueQueueFamillyIndex == queueFamilyIndices[VulkanQueueType::Present] ? VulkanQueueType::Present : VulkanQueueType::COUNT
					// Not using compute nor transfer yet
					// uniqueQueueFamillyIndex == queueFamilyIndices[VulkanQueueType::Compute] ? VulkanQueueType::Compute :
					// uniqueQueueFamillyIndex == queueFamilyIndices[VulkanQueueType::Transfer] ? VulkanQueueType::Transfer :
				)
			);
		}
	}
	return (queuesCreateInfo);
}

/* VULKAN QUEUE TYPE NAMESPACE */

const char* VulkanQueueType::ToString(Type vulkanQueueType)
{
	switch (vulkanQueueType)
	{
	case Type::Graphic:
		return ("Graphic");
	case Type::Present:
		return ("Present");
	/* Not using compute nor transfer yet
	case Type::Compute:
		return ("Compute");
	case Type::Transfer:
		return ("Transfer");
	*/
	default:
		return ("Unknown");
	}
}

VulkanMemoryRequirementsExtended VulkanDeviceHandler::FindMemoryRequirement(const vk::Buffer& buffer, vk::MemoryPropertyFlags memoryProperty) const
{
	VulkanMemoryRequirementsExtended memoryRequirements = static_cast<VulkanMemoryRequirementsExtended>(m_Device.getBufferMemoryRequirements(buffer));
	memoryRequirements.MemoryTypeIndex = FindMemoryTypeIndex(memoryRequirements, memoryProperty);

	return (memoryRequirements);
}

VulkanMemoryRequirementsExtended VulkanDeviceHandler::FindMemoryRequirement(const vk::Image& image, vk::MemoryPropertyFlags memoryProperty) const
{
	VulkanMemoryRequirementsExtended memoryRequirements = static_cast<VulkanMemoryRequirementsExtended>(m_Device.getImageMemoryRequirements(image));
	memoryRequirements.MemoryTypeIndex = FindMemoryTypeIndex(memoryRequirements, memoryProperty);

	return (memoryRequirements);
}

uint32_t VulkanDeviceHandler::FindMemoryTypeIndex(const VulkanMemoryRequirementsExtended& memoryRequirements, vk::MemoryPropertyFlags memoryProperty) const
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = GetPhysicalDeviceMemoryProperties();

	uint32_t memoryTypeIndex = -1;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (memoryRequirements.memoryTypeBits & (1 << i))
		{
			if (memoryProperties.memoryTypes[i].propertyFlags & memoryProperty)
			{
				memoryTypeIndex = i;
				break;
			}
		}
	}
	return (memoryTypeIndex);
}