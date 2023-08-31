#pragma once

#include "Vulkan_API.h"

#include <vulkan/vulkan.hpp>

class VULKAN_API VulkanSwapchainFrame
{
public:
	using AxisSize = uint16_t; // Better to keep in sync with Window::AxisSize (Window.h)

	struct Syncronization
	{
		vk::Semaphore AcquireImage; // Semaphore used to signal that the image is available to be drawn to
		vk::Semaphore RenderFinished; // Semaphore used to signal that the image is ready to be presented
		vk::Fence RenderedFinished; // Fence used to make sure that the image is not being used by the GPU
	};

public:
	VulkanSwapchainFrame()
		: m_Image(VK_NULL_HANDLE),
		m_ImageView(VK_NULL_HANDLE),
		m_CommandBuffer(VK_NULL_HANDLE),
		m_Width(0),
		m_Height(0),
		m_Sync()
	{}
	VulkanSwapchainFrame(
		vk::Image& image,
		AxisSize width,
		AxisSize height,
		vk::Format imageFormat,
		vk::CommandPool& swapchainCmdPool
	);

public:
	virtual void Begin() const;
	virtual void End() const;

public:
	__forceinline AxisSize GetWidth() const { return m_Width; }
	__forceinline AxisSize GetHeight() const { return m_Height; }
	__forceinline vk::Extent2D GetExtent() const { return vk::Extent2D(m_Width, m_Height); }

	__forceinline const vk::Image& GetImage() const { return m_Image; }
	__forceinline const vk::ImageView& GetImageView() const { return m_ImageView; }
	__forceinline const vk::CommandBuffer& GetCommandBuffer() const { return m_CommandBuffer; }
	__forceinline const Syncronization& GetSync() const { return m_Sync; }

protected:
	virtual void Destroy(const vk::CommandPool commandPool);
	virtual void Resize(vk::Image& image, AxisSize width, AxisSize height, vk::Format imageFormat);

protected:
	// The image that is used as a frame
	vk::Image m_Image;
	vk::ImageView m_ImageView;
	// Command buffer, used to record the command to draw onto the frame (m_Image)
	vk::CommandBuffer m_CommandBuffer;

	AxisSize m_Width;
	AxisSize m_Height;

	// Syncronization object, used to synchronize the job of the GPU and the CPU
	// E.g: Making sure that the image is acquired before drawing to it
	Syncronization m_Sync;

	friend class VulkanSwapchain;
};
