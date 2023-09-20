#pragma once

#include "Vulkan_API.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;

/**
 * Vulkan context, used to initialize the Vulkan API and store all the global vulkan objects.
 * Those object will be directly access by vulkan implementations, to avoid passing them as parameters.
 */
class VULKAN_API VulkanContext
{
private:
	VulkanContext() = default;
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext operator=(const VulkanContext&) = delete;
	~VulkanContext();

public:
	static VulkanContext& Get()
	{
		static VulkanContext Instance;
		return Instance;
	}

public:
	/**
	 * Initialize the vulkan instance.
	 *
	 * @param major The major Vulkan version to use, if UINT32_MAX, the latest version will be used.
	 * @param minor The minor Vulkan version to use, if UINT32_MAX, the latest version will be used.
	 * @param patch The patch Vulkan version to use, if UINT32_MAX, the latest version will be used.
	 * @return true if the instance was or were already created, false otherwise.
	 */
	bool CreateInstance(uint32_t major = UINT32_MAX, uint32_t minor = UINT32_MAX, uint32_t patch = UINT32_MAX);

	/**
	 * Initialize the vulkan device.
	 *
	 * @param surface The surface on which the device will draw on
	 * @return true if the device was or were already created, false otherwise.
	 */
	bool CreateDevice(vk::SurfaceKHR& surface, vk::PhysicalDeviceFeatures features = {});

	__forceinline void AddInstanceLayer(const char* layerName) { m_Instance.AddLayer(layerName); }
	__forceinline void AddInstanceExtension(const char* extensionName) { m_Instance.AddExtension(extensionName); }
	__forceinline void AddDeviceExtension(const char* extensionName, void* feature = nullptr) { m_Device.AddExtension(extensionName, feature); }

public:
	__forceinline static const VulkanInstance& GetInstance() { return Get().m_Instance; }
	__forceinline static const VulkanDevice& GetDevice() { return Get().m_Device; }
	__forceinline static const vk::PhysicalDevice& GetPhysicalDevice() { return Get().m_Device.GetPhysicalDevice(); }
	__forceinline static const vk::DispatchLoaderDynamic& GetDispatcher() { return Get().m_Dispatcher; }

protected:
	void Destroy();

protected:
	VulkanInstance m_Instance;
	VulkanDevice m_Device;

	vk::DispatchLoaderDynamic m_Dispatcher;
#ifdef OV_DEBUG
	vk::DebugUtilsMessengerEXT m_DebugMessenger;
#endif // OV_DEBUG

	friend class VulkanModule;
};
