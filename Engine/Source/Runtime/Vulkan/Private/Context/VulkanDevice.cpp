#include "VulkanDevice.h"
#include "Vulkan/Log.h"
#include "Vulkan/ErrorHandling.h"
#include "VulkanContext.h"

#include <set>

bool VulkanDevice::Create(vk::SurfaceKHR& surface, vk::PhysicalDevice* physicalDevice)
{
	if (physicalDevice && IsASuitableDevice(*physicalDevice))
		m_PhysicalDevice = *physicalDevice;

	// If you didn't specified any physical device (or the one you specified was not suitable)
	// we use the first one that is suitable
	if (physicalDevice == VK_NULL_HANDLE)
	{
		auto suitableDevices = FetchAllSuitablePhysicalDevices();
		if (suitableDevices.empty())
		{
			VULKAN_LOG(Error, "No suitable devices found");
			return (false);
		}
		m_PhysicalDevice = suitableDevices[0]; // Use the first suitable device
	}

#ifdef WITH_LOGGING
	vk::PhysicalDeviceProperties deviceProperties = m_PhysicalDevice.getProperties();
	VULKAN_LOG(Verbose, "Using device: \"{:s}\" ({:s})",
		deviceProperties.deviceName.data(), vk::to_string(deviceProperties.deviceType)
	);
#endif // WITH_LOGGING

	VULKAN_LOG(Verbose, "Device extensions:");
	VULKAN_LOG_ARRAY(m_ExtensionNames, Verbose, "\t\"{:s}\"");

	FindQueueFamilyIndicies(surface);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		m_QueueFamilyIndicies.Graphics,
		m_QueueFamilyIndicies.Present,
		m_QueueFamilyIndicies.Compute,
		m_QueueFamilyIndicies.Transfer
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo(
			vk::DeviceQueueCreateFlags(),
			queueFamily,
			1,
			&queuePriority
		);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures physicalDeviceFeatures = {};
	vk::DeviceCreateInfo createInfo(
		vk::DeviceCreateFlags(),
		static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data(),
		0, nullptr, // Device layers are deprecated since Vulkan 1.0.13
		static_cast<uint32_t>(m_ExtensionNames.size()), m_ExtensionNames.data(),
		&physicalDeviceFeatures,
		m_FeatureChain.Begin()
	);

	try {
		vk::Device logicalDevice = m_PhysicalDevice.createDevice(createInfo);
		*this = VulkanDevice(logicalDevice, this);
		return (true);
	}
	catch (vk::SystemError& e)
	{
		VULKAN_LOG(Error, "Failed to create logical device: \"{:s}\"", e.what());
		return (false);
	}
}

void VulkanDevice::Destroy()
{
	if (this->operator bool())
	{
		waitIdle();

		VULKAN_LOG(Verbose, "Destroying logical device...");
		destroy();

		*this = VulkanDevice(); // Reset the device
	}
}

std::vector<vk::PhysicalDevice> VulkanDevice::FetchAllSuitablePhysicalDevices() const
{
	std::vector<vk::PhysicalDevice> suitableDevices;

	auto allDevices = VulkanContext::GetInstance().enumeratePhysicalDevices();
	for (const auto& device : allDevices)
	{
#ifdef WITH_LOGGING
		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		VULKAN_LOG(VeryVerbose, "Check device: \"{:s}\" ({:s})",
			deviceProperties.deviceName.data(), vk::to_string(deviceProperties.deviceType)
		);
#endif // WITH_LOGGING
		if (IsASuitableDevice(device))
		{
			VULKAN_LOG(VeryVerbose, "\tIs suitable");
			suitableDevices.push_back(device);
		}
		else
			VULKAN_LOG(VeryVerbose, "\tIs not suitable");
	}
	return (suitableDevices);
}

void VulkanDevice::SubmitOneTimeCommandBuffer(uint8_t queueFamilyIndex, std::function<void(vk::CommandBuffer &)> lambda) const
{
	if (queueFamilyIndex == UINT8_MAX)
	{
		VULKAN_LOG(Error, "Invalid queue family index");
		return;
	}
	if (lambda == nullptr)
	{
		VULKAN_LOG(Error, "Invalid lambda function");
		return;
	}

	vk::CommandPoolCreateInfo poolInfo(
	vk::CommandPoolCreateFlagBits::eTransient,
		queueFamilyIndex
	);
	vk::CommandPool commandPool = createCommandPool(poolInfo);

	vk::FenceCreateInfo fenceInfo(
		vk::FenceCreateFlagBits::eSignaled
	);
	vk::Fence fence = createFence(fenceInfo);

	vk::CommandBuffer commandBuffer = allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(
			commandPool,
			vk::CommandBufferLevel::ePrimary,
			1
		)
	)[0];

	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
	lambda(commandBuffer);
	commandBuffer.end();

	resetFences(fence);
	vk::SubmitInfo submitInfo(
		0, nullptr,
		nullptr,
		1, &commandBuffer,
		0, nullptr
	);
	getQueue(queueFamilyIndex, 0).submit(submitInfo, fence);
	waitForFences(fence, VK_TRUE, UINT64_MAX);

	destroyFence(fence);
	freeCommandBuffers(commandPool, 1, &commandBuffer);
	destroyCommandPool(commandPool);
}

void VulkanDevice::FindQueueFamilyIndicies(vk::SurfaceKHR &surface)
{
	m_QueueFamilyIndicies = QueueFamily<uint8_t>(UINT8_MAX);

	auto queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();

	VULKAN_LOG(VeryVerbose, "\tQueueFamily | Graphics | Compute | Transfer | Present | QueueCount");

	QueueFamily<uint8_t> queueFamilyScore(UINT8_MAX);
	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		uint8_t currentQueueFamilyScore = 0;

		// Check is the queue can present to the surface
		uint32_t isSurfaceSupported = 0;
		CHECK_VK_RESULT(
			m_PhysicalDevice.getSurfaceSupportKHR(i, surface, &isSurfaceSupported),
			"Check if a family queue support presenting", Warning
		);

		// Log the queue family infos
		VULKAN_LOG(VeryVerbose, "\t{:11d} | {:>8s} | {:>7s} | {:>8s} | {:>7s} | {:10d}",
			i,
			queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics ? "x" : "",
			queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute ? "x" : "",
			queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer ? "x" : "",
			m_PhysicalDevice.getSurfaceSupportKHR(i, surface) ? "x" : "",
			queueFamilyProperties[i].queueCount
		);

		// We compute a score based on the queue family polyvalence
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			currentQueueFamilyScore++;
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
			currentQueueFamilyScore++;
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer)
			currentQueueFamilyScore++;
		if (isSurfaceSupported == 1)
			currentQueueFamilyScore++;

		currentQueueFamilyScore -= (currentQueueFamilyScore <= queueFamilyProperties[i].queueCount ? 0 : queueFamilyProperties[i].queueCount / 4);

		currentQueueFamilyScore = 0;

		// We set this queue family as the default one if it has the smallest score
		// We do this to use the queue family that is the most specialized
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics
			&& currentQueueFamilyScore < queueFamilyScore.Graphics)
		{
			queueFamilyScore.Graphics = currentQueueFamilyScore;
			m_QueueFamilyIndicies.Graphics = i;
		}
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute
			&& currentQueueFamilyScore < queueFamilyScore.Compute)
		{
			queueFamilyScore.Compute = currentQueueFamilyScore;
			m_QueueFamilyIndicies.Compute = i;
		}
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer
			&& currentQueueFamilyScore < queueFamilyScore.Transfer)
		{
			queueFamilyScore.Transfer = currentQueueFamilyScore;
			m_QueueFamilyIndicies.Transfer = i;
		}
		if (isSurfaceSupported && currentQueueFamilyScore < queueFamilyScore.Present)
		{
			queueFamilyScore.Present = currentQueueFamilyScore;
			m_QueueFamilyIndicies.Present = i;
		}
	}

	// Those log will appear on the console, the previous one will only appear in the log file
	VULKAN_LOG(Verbose, "\tQueueFamily | Graphics | Compute | Transfer | Present");
	VULKAN_LOG(Verbose, "\t            | {:8d} | {:7d} | {:8d} | {:7d}",
		m_QueueFamilyIndicies.Graphics == UINT8_MAX ? -1 : m_QueueFamilyIndicies.Graphics,
		m_QueueFamilyIndicies.Compute == UINT8_MAX ? -1 : m_QueueFamilyIndicies.Compute,
		m_QueueFamilyIndicies.Transfer == UINT8_MAX ? -1 : m_QueueFamilyIndicies.Transfer,
		m_QueueFamilyIndicies.Present == UINT8_MAX ? -1 : m_QueueFamilyIndicies.Present
	);
}

bool VulkanDevice::IsASuitableDevice(const vk::PhysicalDevice &physicalDevice) const
{
	return (
		SupportAllExtensions(physicalDevice)
	);
}

bool VulkanDevice::SupportAllExtensions(const vk::PhysicalDevice& device) const
{
	auto extensionProperties = device.enumerateDeviceExtensionProperties();

#ifdef WITH_LOGGING
	bool areAllExtensionSupported = true;

	// Loop over all the requested extensions, to see if any of them are not supported
	for (const char* extensionName : m_ExtensionNames)
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
			if (areAllExtensionSupported) // Will only happen for the first unsupported extension
			{
				areAllExtensionSupported = false;
				// VeryVerbose because if I have 10 GPU and only the last one is suitable it's gonna flood the console.
				// But if none is suitable you'll be have a error message and will be able to look into the log file
				// to see on a per GPU basis why are they not suitable.
				VULKAN_LOG(VeryVerbose, "Unsupported device extension(s):");
			}
			VULKAN_LOG(VeryVerbose, "\t{:s}", extensionName);
			// Don't return yet to list all unsupported extensions
		}
	}
	return (areAllExtensionSupported);
#else
	// Loop over all the requested extension if one of them is not found return false
	for (const char* extensionName : m_ExtensionNames)
	{
		for (const auto& extensionProperty : extensionProperties)
		{
			if (strcmp(extensionName, extensionProperty.extensionName) != 0 /* not the same */)
				return (false);
		}
	}
	return (true);
#endif // WITH_LOGGING
}

void VulkanDevice::AddExtension(const char *extensionName, void *feature)
{
	m_ExtensionNames.push_back(extensionName);
	if (feature != nullptr)
		m_FeatureChain.PushBack(feature);
}
