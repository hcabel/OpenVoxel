#pragma once

#include "Engine.h"
#include "Vulkan/VulkanImpl.h"
#include <vulkan/vulkan.hpp>

class ENGINE_API VulkanSwapchain final
{
#pragma region Internal Struct
public:
	struct SupportProperties
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	struct Frame
	{
		vk::Image Image;
		vk::ImageView View;
		vk::CommandBuffer CommandBuffer;
		const uint32_t Index;

		Frame(vk::Image image, vk::ImageView view, vk::CommandBuffer commandBuffer, uint32_t index)
			: Image(image), View(view), CommandBuffer(commandBuffer), Index(index)
		{}
	};
#pragma endregion

public:
	VulkanSwapchain(const Vulkan::DeviceBundle& device, const vk::SurfaceKHR& surface, const vk::CommandPool& commandPool, vk::PresentModeKHR preferredPresentMode = vk::PresentModeKHR::eMailbox);
	~VulkanSwapchain();

#pragma region Methods
public:
	/** Get a new frame that is not currently presented by the screen. */
	Frame& GetNextFrame();
	/**
	 * Request to present a new frame on screen. (does not guarantee that the frame will be presented on screen, it depend on the swap chain present mode)
	 *
	 * \param frame The frame to present
	 */
	void PresentFrame(const Frame& frame);

private:
	SupportProperties RequestSwapchainProperties(const vk::SurfaceKHR& surface);
	/**
	 * Select a surface format that is supported by the device, if the preferred format is not supported then the first available format is used.
	 *
	 * \param availableFormats A list of all the surface format that are supported by the device.
	 *
	 * \param preferredFormat The format that you wish to use.
	 * \param preferredColorSpace The color space that you wish to use.
	 * \return your preferred format if supported otherwise the first available format in the list.
	 */
	vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> availableFormats, vk::Format preferredFormat = vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR preferredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear) const;
	/**
	 * Select a present mode than is supported by the device, if the preferred present mode is not supported then the FIFO mode is used.
	 *
	 * \param availablePresentModeList A list of all the present mode that are supported by the device.
	 * \param preferredPresentMode The present mode that you wish to use.
	 * \return your preferred present mode if supported otherwise the FIFO because it's required to be supported
	 */
	vk::PresentModeKHR SelectPresentMode(std::vector<vk::PresentModeKHR> availablePresentModeList, vk::PresentModeKHR preferredPresentMode = vk::PresentModeKHR::eMailbox) const;
	/**
	 * Selects the optimal size of the swap chain images for presenting to the screen.
	 * If current extend is not set will return the median of the min and max extent. TODO: test that to see how it behave
	 *
	 * \param capabilities A reference to a structure describing the capabilities of the presentation surface.
	 * \return A `vk::Extent2D` object representing the optimal size of the swap chain images.
	 */
	vk::Extent2D SelectExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;
	Frame CreateFrame(uint32_t frameIndex, vk::Image& image, const vk::CommandPool& commandPool);

#pragma endregion

#pragma region Properties
private:
	const Vulkan::DeviceBundle& m_Device;
	const vk::CommandPool& m_CommandPool;

	uint32_t m_CurrentFrameIndex = 0;

	// Signaled when the image has be acquired and is ready for rendering.
	vk::Semaphore m_ImageAvailableSemaphore;
	// Signaled when the image has been rendered to and is ready to be presented. (GPU side)
	vk::Semaphore m_RenderFinishedSemaphore;
	// Signaled when the image has been rendered to and is ready to be presented. (CPU side)
	vk::Fence m_RenderFinishedFence;

	vk::SwapchainKHR m_Swapchain;
	vk::Extent2D m_Extent;
	vk::SurfaceFormatKHR m_SurfaceFormat;
	vk::PresentModeKHR m_PresentMode;

	std::vector<Frame> m_Frames;
#pragma endregion

};
