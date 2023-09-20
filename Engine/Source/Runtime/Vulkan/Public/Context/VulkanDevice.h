#pragma once

#include "Vulkan_API.h"
#include "VulkanPNextChain.h"

#include <vector>
#include <vulkan/vulkan.hpp>
#include <functional>

// This class is exposed public because it is used in the VulkanContext class
// but in theory it will be nice if it wasn't
class VULKAN_API VulkanDevice : public vk::Device
{
public:
	template<typename T>
	struct QueueFamily
	{
		T Graphics;
		T Present;
		T Compute;
		T Transfer;

		QueueFamily() = default;
		QueueFamily(T defaultValue) // One value for all
			: Graphics(defaultValue), Present(defaultValue), Compute(defaultValue), Transfer(defaultValue)
		{}
		QueueFamily(T graphics, T present, T compute, T transfer) // One value for each
			: Graphics(graphics), Present(present), Compute(compute), Transfer(transfer)
		{}
		QueueFamily(QueueFamily<T>&& rhs) noexcept
			: Graphics(std::move(rhs.Graphics)),
			Present(std::move(rhs.Present)),
			Compute(std::move(rhs.Compute)),
			Transfer(std::move(rhs.Transfer))
		{}
		QueueFamily& operator=(QueueFamily<T>&& rhs) noexcept
		{
			Graphics = std::move(rhs.Graphics);
			Present = std::move(rhs.Present);
			Compute = std::move(rhs.Compute);
			Transfer = std::move(rhs.Transfer);
			return (*this);
		}
	};

protected:
	VulkanDevice& operator=(VulkanDevice* rhs)
	{
		m_ExtensionNames = std::move(rhs->m_ExtensionNames);
		m_FeatureChain = std::move(rhs->m_FeatureChain);
		m_QueueFamilyIndicies = std::move(rhs->m_QueueFamilyIndicies);
		m_PhysicalDevice = std::move(rhs->m_PhysicalDevice);
		return (*this);
	}

protected:
	VulkanDevice()
		: vk::Device(VK_NULL_HANDLE),
		m_ExtensionNames(),
		m_FeatureChain(),
		m_QueueFamilyIndicies(),
		m_PhysicalDevice(VK_NULL_HANDLE)
	{}
	VulkanDevice(vk::Device device, VulkanDevice* rhs)
		: vk::Device(device),
		m_ExtensionNames(std::move(rhs->m_ExtensionNames)),
		m_FeatureChain(std::move(rhs->m_FeatureChain)),
		m_QueueFamilyIndicies(std::move(rhs->m_QueueFamilyIndicies)),
		m_PhysicalDevice(rhs->m_PhysicalDevice)
	{}

public:
	/**
	 * Create a logical device.
	 * @note This function will select a default physical device if you didn't specify one.
	 *
	 * @return True if the device was created successfully, false otherwise.
	 */
	bool Create(vk::SurfaceKHR& surface, vk::PhysicalDevice* physicalDevice = VK_NULL_HANDLE, vk::PhysicalDeviceFeatures features = {});
	void Destroy();

	/**
	 * Search for a suitable physical device.
	 * @note This function will check extension and layer support. (so add your extensions and layers before calling this function)
	 *
	 * @return A vector of all suitable physical devices.
	 */
	std::vector<vk::PhysicalDevice> FetchAllSuitablePhysicalDevices() const;

	/**
	 * Create, record and send command buffer, use for one time commands.
	 * To record the command buffer, use the lambda function.
	 * @note: This function is waiting for the command buffer to finish executing before returning.
	 *
	 * @param queueFamilyIndex The queue family index to use.
	 * @param lambda The lambda function to record the command buffer.
	 */
	void SubmitOneTimeCommandBuffer(uint8_t queueFamilyIndex, std::function<void(vk::CommandBuffer&)> lambda) const;

public:
	__forceinline const vk::PhysicalDevice& GetPhysicalDevice() const { return (m_PhysicalDevice); }
	__forceinline const QueueFamily<uint8_t>& GetQueueFamilyIndicies() const { return (m_QueueFamilyIndicies); }

protected:
	void FindQueueFamilyIndicies(vk::SurfaceKHR& surface);
	bool IsASuitableDevice(const vk::PhysicalDevice& physicalDevice) const;
	bool SupportAllExtensions(const vk::PhysicalDevice& device) const;

public:
	__forceinline void AddExtension(const char* extensionName, void* feature = nullptr);

protected:
	std::vector<const char*> m_ExtensionNames;
	VulkanPNextChain m_FeatureChain;

	QueueFamily<uint8_t> m_QueueFamilyIndicies;

	vk::PhysicalDevice m_PhysicalDevice;

	friend class VulkanContext;
};
