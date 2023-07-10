#pragma once

#include "Renderer_API.h"
#include "vulkan/VulkanUtils.h"

#include <vulkan/vulkan.hpp>

class VulkanDeviceHandler;

struct VulkanSwapChainSupportProperties
{
	vk::SurfaceCapabilitiesKHR Capabilities;
	std::vector<vk::SurfaceFormatKHR> Formats;
	std::vector<vk::PresentModeKHR> PresentModes;
};

struct VulkanSwapChainFrame
{
	vk::Image Image;
	vk::CommandBuffer CommandBuffer;
	vk::Fence RenderedFence;
	vk::Semaphore RenderedSemaphore;
	vk::Semaphore AcquiredSemaphore;
};

class RENDERER_API VulkanSwapChainHandler final
{

public:
	VulkanSwapChainHandler() = default;
	VulkanSwapChainHandler(const VulkanDeviceHandler* vkDevice, const vk::SurfaceKHR* surface);
	~VulkanSwapChainHandler() = default;

	VulkanSwapChainHandler(const VulkanSwapChainHandler& rhs) = delete;
	VulkanSwapChainHandler(VulkanSwapChainHandler&& rhs) noexcept = delete;
	VulkanSwapChainHandler operator=(const VulkanSwapChainHandler& rhs) = delete;
	VulkanSwapChainHandler operator=(VulkanSwapChainHandler&& rhs) noexcept = delete;

	operator vk::SwapchainKHR() const { return Raw(); }

public:
	/**
	 * Create the swap chain.
	 *
	 * \param prefferedPresentMode The preferred present mode to use (may not be available)
	 */
	void CreateSwapChain(vk::PresentModeKHR preferredPresentMode = vk::PresentModeKHR::eFifo);
	/** Destroy/cleanup the swap chain */
	void DestroySwapChain();

	/** Acquire the next image in the swap chain. */
	uint8_t AcquireNextFrameIndex();
	/** Render frame */
	void SubmitWork(uint8_t frameIndex) const;
	/** Present frame */
	void PresentFrame(uint8_t frameIndex);

private:
	VulkanSwapChainSupportProperties RequestSwapchainProperties() const;
	/**
	 * Select a surface format that is supported by the device, if the preferred format is not supported then the first available format is used.
	 *
	 * \param availableFormats A list of all the surface format that are supported by the device.
	 *
	 * \param preferredFormat The format that you wish to use.
	 * \param preferredColorSpace The color space that you wish to use.
	 * \return your preferred format if supported otherwise the first available format in the list.
	 */
	vk::SurfaceFormatKHR SelectSurfaceFormat(
		const std::vector<vk::SurfaceFormatKHR> availableFormats,
		vk::Format preferredFormat = vk::Format::eR8G8B8A8Srgb,
		vk::ColorSpaceKHR preferredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear) const;
	/**
	 * Select a present mode than is supported by the device, if the preferred present mode is not supported then the FIFO mode is used.
	 *
	 * \param availablePresentModeList A list of all the present mode that are supported by the device.
	 * \param preferredPresentMode The present mode that you wish to use.
	 * \return your preferred present mode if supported otherwise the FIFO because it's required to be supported
	 */
	vk::PresentModeKHR SelectPresentMode(
		std::vector<vk::PresentModeKHR> availablePresentModeList,
		vk::PresentModeKHR preferredPresentMode = vk::PresentModeKHR::eFifo) const;
	/**
	 * Selects the optimal size of the swap chain images for presenting to the screen.
	 * If current extend is not set will return the median of the min and max extent. TODO: test that to see how it behave
	 *
	 * \param capabilities A reference to a structure describing the capabilities of the presentation surface.
	 * \return A `vk::Extent2D` object representing the optimal size of the swap chain images.
	 */
	vk::Extent2D SelectExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

	VulkanSwapChainFrame CreateFrame(const vk::Image& image);

public:
	/** Get the vulkan swap chain */
	vk::SwapchainKHR Raw() const { return m_Swapchain; }

	/** Get the size of the swap chain */
	vk::Extent2D GetExtent() const { return m_Extent; }
	/** Get the swap chain image format */
	vk::Format GetFormat() const { return m_SurfaceFormat.format; }

	/** Connect the swap chain to a device */
	void SetVulkanDevice(const VulkanDeviceHandler* vkDevice) { m_VkDevice = vkDevice; }
	/** Connect the swap chain to a surface */
	void SetSurface(const vk::SurfaceKHR* surface) { m_Surface = surface; }
	/** Connect the swap chain to a command pool */
	void SetCommandPool(const vk::CommandPool* commandPool) { m_CommandPool = commandPool; }

	const VulkanSwapChainFrame& GetFrame(uint8_t frameIndex) const { return m_Frames[frameIndex]; }
	const std::vector<VulkanSwapChainFrame>& GetFrames() const { return m_Frames; }

private:
	const VulkanDeviceHandler* m_VkDevice = nullptr;
	const vk::SurfaceKHR* m_Surface = nullptr;
	const vk::CommandPool* m_CommandPool = nullptr;
	/* Whether or not the swap chain has been created */
	bool m_IsSwapChainCreated = false;

	/** An index that keep track of which frame is the next to be rendered */
	uint8_t m_CurrentFrameIndex;
	uint8_t m_CurrentSemaphoreIndex = 0;
	uint8_t m_ImageCount;

	/** The vulkan swap chain */
	vk::SwapchainKHR m_Swapchain;
	/** The size of the swap chain */
	vk::Extent2D m_Extent;
	/** The surface format of the swap chain */
	vk::SurfaceFormatKHR m_SurfaceFormat;
	/** The present mode of the swap chain */
	vk::PresentModeKHR m_PresentMode = vk::PresentModeKHR::eFifo;
	/** All the frame in the swap chain */
	std::vector<VulkanSwapChainFrame> m_Frames;
};
