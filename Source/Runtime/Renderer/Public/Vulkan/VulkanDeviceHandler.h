#pragma once

#include "Renderer_API.h"
#include "VulkanNextChain.h"
#include "Version.h"

#include <vulkan/vulkan.hpp>
#include <memory>

class VulkanInstanceHandler;

namespace VulkanQueueType
{
	enum Type : uint8_t
	{
		Graphic = 0,
		Present = 1,
		// Compute = 1, // Not using compute nor transfer yet
		// Transfer = 2,

		COUNT = 2
	};

	const char* ToString(Type vulkanQueueType);
}

struct VulkanMemoryRequirementsExtended : public vk::MemoryRequirements
{
	uint32_t MemoryTypeIndex = -1;
};

class RENDERER_API VulkanDeviceHandler final
{

public:
	struct VulkanQueue : public vk::Queue
	{
		uint32_t FamilyIndex = 0;

		VulkanQueue()
			: vk::Queue(), FamilyIndex(UINT32_MAX)
		{}
		VulkanQueue(vk::Queue queue)
			: vk::Queue(queue), FamilyIndex(UINT32_MAX)
		{}
	};

public:
	VulkanDeviceHandler() = default;
	VulkanDeviceHandler(const VulkanInstanceHandler* instance);
	~VulkanDeviceHandler() = default;

	VulkanDeviceHandler(const VulkanDeviceHandler& rhs) = delete;
	VulkanDeviceHandler(VulkanDeviceHandler&& rhs) noexcept = delete;
	VulkanDeviceHandler operator=(const VulkanDeviceHandler& rhs) = delete;
	VulkanDeviceHandler operator=(VulkanDeviceHandler&& rhs) noexcept = delete;

	__forceinline operator vk::Device() const { return Raw(); }
	__forceinline operator VkDevice() const { return Raw(); }
	__forceinline operator vk::PhysicalDevice() const { return GetPhysicalDevice(); }

public:
	/**
	 * Create the Logical device to handle the physical device.
	 * If you didn't set any physical device, it will use the most suited one.
	 */
	void CreateDevice(const vk::SurfaceKHR& surface);
	/** Destroy the logical device */
	void DestroyDevice();

	/** Add an extension to the vulkan device */
	void AddExtension(const char* extension, void* feature = nullptr);
	/** Add a layer to the vulkan device */
	void AddLayer(const char* layer);

	VulkanMemoryRequirementsExtended FindMemoryRequirement(const vk::Buffer& buffer, vk::MemoryPropertyFlags memoryProperty) const;
	VulkanMemoryRequirementsExtended FindMemoryRequirement(const vk::Image& image, vk::MemoryPropertyFlags memoryProperty) const;

private:
	uint32_t FindMemoryTypeIndex(const VulkanMemoryRequirementsExtended& memoryRequirements, vk::MemoryPropertyFlags memoryProperty) const;
	/**
	 * Get a suitable physical device.
	 * \return Suitable physical device
	 */
	vk::PhysicalDevice FetchSuitablePhysicalDevice() const;
	/**
	 * Check if the physical device is suitable for the application.
	 * \param physicalDevice The physical device to check
	 * \return True if the physical device is suitable, false otherwise
	 */
	bool IsDeviceSuitable(vk::PhysicalDevice physicalDevice) const;
	/** Tell whether or not all the vulkan device extension in m_Extension are supported */
	bool allExtensionsAreSupported() const;
	/** Tell whether or not all the vulkan device layers in m_Layers are supported */
	bool allLayersAreSupported() const;

	std::array<uint32_t, VulkanQueueType::COUNT> FindQueueFamilyIndices(const vk::SurfaceKHR& surface) const;
	std::vector<vk::DeviceQueueCreateInfo> SetupQueuesCreateInfo(const std::array<uint32_t, VulkanQueueType::COUNT>& queueFamilyIndices) const;

public:
	/** Set the reference to the vulkan instance */
	void SetVulkanInstance(const VulkanInstanceHandler* vkInstance) { m_VkInstance = vkInstance; }
	/** Set the physical device (GPU) to use */
	void SetPhysicalDevice(vk::PhysicalDevice physicalDevice) { m_PhysicalDevice = physicalDevice; }

	__forceinline const vk::Device& Raw() const { return (m_Device); }
	/** Get Physical device raw class (C++ API) */
	__forceinline const vk::PhysicalDevice& GetPhysicalDevice() const { return (m_PhysicalDevice); }
	/** Get the queue of the specified type */
	__forceinline const VulkanQueue& GetQueue(VulkanQueueType::Type type) const { return (m_Queues[type]); }
	/** Get all the queues */
	__forceinline const std::array<VulkanQueue, VulkanQueueType::COUNT>& GetQueues() const { return (m_Queues); }

	__forceinline const vk::PhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return (m_PhysicalDeviceProperties); }
	__forceinline const vk::PhysicalDeviceProperties2& GetPhysicalDeviceProperties2() const { return (m_PhysicalDeviceProperties2); }
	__forceinline const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& GetRaytracingProperties() const { return (m_RaytracingProperties); }

	vk::PhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() const { return (m_PhysicalDevice.getMemoryProperties()); }

private:
	// pointer to the vulkan instance
	const VulkanInstanceHandler* m_VkInstance = nullptr;
	// Whether or not the logical device has been created
	bool m_IsDeviceCreated = false;

	// The vulkan device extensions that you wish to use
	std::vector<const char*> m_Extensions;
	VulkanNextChain m_FeatureChain;
	// The vulkan device layers that you wish to use
 	std::vector<const char*> m_Layers;

	// The logical device that handle the physical device
	vk::Device m_Device;
	// The physical device that correspond to one of your GPU
	vk::PhysicalDevice m_PhysicalDevice;
	std::array<VulkanQueue, VulkanQueueType::COUNT> m_Queues;

	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_RaytracingProperties;
	vk::PhysicalDeviceProperties m_PhysicalDeviceProperties;
	vk::PhysicalDeviceProperties2 m_PhysicalDeviceProperties2;

};
