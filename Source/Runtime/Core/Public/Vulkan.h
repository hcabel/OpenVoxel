#pragma once

#include "Logging/LoggingMacros.h"
#include <vulkan/vulkan.hpp>
#include <optional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

DECLARE_LOG_CATEGORY(LogVulkan)

namespace QueueType
{
	enum Type : uint8_t
	{
		Graphics = 0,
		Present = 1,
	};

	constexpr uint8_t Count = 2;
}

/**
 * An helper class to setup vulkan.
 * This is a static class and should not store any data
 */
class Vulkan final
{
public:
	Vulkan() = delete;
	~Vulkan() = delete;

public:
#if OV_DEBUG
	struct DebugMessenger
	{
		vk::DebugUtilsMessengerEXT Messenger;
		vk::DispatchLoaderDynamic Dldi;
	};
#endif

	struct DeviceBundle
	{
		vk::PhysicalDevice Physical;
		vk::Device Logical;
		std::array<vk::Queue, QueueType::Count> Queues;
	};

	struct QueueFamilyIndices
	{
		uint32_t Graphics = UINT32_MAX;
		uint32_t Present = UINT32_MAX;

		bool IsComplete() const { return (Graphics != UINT32_MAX) && (Present != UINT32_MAX); }
	};

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	struct SwapChainFrame
	{
		vk::Image Image;
		vk::ImageView View;
	};

	struct SwapChainBundle
	{
		vk::SwapchainKHR SwapChain;
		std::vector<SwapChainFrame> Frames;
		vk::Format SurfaceFormat;
		vk::Extent2D Extent;
	};

public:
	/**
	 * Create a fully initialized Vulkan instance.
	 * You can pass extra layers and extension if you wish.
	 *
	 * \param additionalExtensions the extra extensions that you want to add
	 * \param additionalLayers the extra layers that you want to add
	 * \return the vulkan instance
	 */
	static std::optional<vk::Instance> CreateInstance(const std::vector<const char*>& additionalExtensions = {}, const std::vector<const char*>& additionalLayers = {}) noexcept;
#if OV_DEBUG
	static DebugMessenger SetupDebugMessenger(const vk::Instance &instance) noexcept;
#endif
	static std::optional<vk::SurfaceKHR> CreateSurface(const vk::Instance instance, GLFWwindow* window) noexcept;
	static std::optional<DeviceBundle> CreateDevice(const vk::Instance &instance, const vk::SurfaceKHR &surface) noexcept;
	static std::optional<SwapChainBundle> CreateSwapChain(const vk::Instance &instance, const DeviceBundle &device, const vk::SurfaceKHR &surface, uint32_t width, uint32_t height) noexcept;

private:
#pragma region VkInstance
	/** Get the Vulkan version that we wish to use */
	static uint32_t GetVulkanVersion();
	/** Get the Vulkan Instance extensions required for the engine to run */
	static std::vector<const char*> GetRequiredInstanceExtensions();
	/**
	 * Check if the extensions are supported.
	 *
	 * \param extensions the extensions that you want to check
	 * \return true if all the extensions are supported, false otherwise
	 */
	static bool IsExtensionsSupported(const std::vector<const char*>& extensions);
	/** Get the Vulkan Instance Layers required for the engine to run */
	static std::vector<const char*> GetRequiredInstanceLayers();
	/**
	 * Check if the layers are supported.
	 *
	 * \param layers the layers that you want to check
	 * \return true if all the layers are supported, false otherwise
	 */
	static bool IsLayersSupported(const std::vector<const char*>& layers);
#pragma endregion

#pragma region Device
	static std::optional<vk::PhysicalDevice> GetPhysicalDevice(const vk::Instance &instance, const std::vector<const char*>& layers, const std::vector<const char*>& extensions) noexcept;
	static std::vector<const char*> GetDeviceLayers() noexcept;
	static std::vector<const char*> GetDeviceExtensions() noexcept;
	static bool IsDeviceSuitable(const vk::PhysicalDevice& device, const std::vector<const char*>& layers, const std::vector<const char*>& extensions) noexcept;
	static std::optional<vk::Device> CreateLogicalDevice(const vk::PhysicalDevice& device, const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const QueueFamilyIndices& familyIndices) noexcept;
#pragma endregion

#pragma region Queue
	static QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface) noexcept;
	static vk::DeviceQueueCreateInfo CreateQueue(const vk::PhysicalDevice &device, uint32_t queueFamilyIndex) noexcept;
#pragma endregion

#pragma region SwapChain
	static SwapChainSupportDetails QuerySwapChainSupport(const DeviceBundle& device, const vk::SurfaceKHR& surface) noexcept;
	static vk::SurfaceFormatKHR SelectSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) noexcept;
	static vk::PresentModeKHR SelectSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) noexcept;
	static vk::Extent2D SelectSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) noexcept;
	static SwapChainFrame CreateSwapChainFrame(const DeviceBundle& device, const vk::Image &image, const vk::Format& imageFormat) noexcept;
#pragma endregion

};
