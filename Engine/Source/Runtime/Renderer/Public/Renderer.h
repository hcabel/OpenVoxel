#pragma once

#include "Renderer_API.h"
#include "Vulkan/VulkanInstanceHandler.h"
#include "Vulkan/VulkanDeviceHandler.h"
#include "Vulkan/VulkanSwapChainHandler.h"
#include "Vulkan/VulkanRayTracingPipeline.h"
#include "Vulkan/VulkanAccelerationStructure.h"
#include "Vulkan/VulkanShaderBindingTable.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <memory>

/**
 * Handle the rendering of each frame on the window.
 */
class RENDERER_API Renderer final
{

public:
	/* DO NOT CALL DIRECTLY, use the Get/Initialize method */
	Renderer(GLFWwindow* window);
	~Renderer();
	// delete copy and move constructors and assign operators
	Renderer(const Renderer&) = delete;             // Copy construct
	Renderer(Renderer&&) = delete;                  // Move construct
	Renderer& operator=(const Renderer&) = delete;  // Copy assign
	Renderer& operator=(Renderer&&) = delete;       // Move assign

	/**
	 * Get The Renderer singleton.
	 *
	 * \param window (optional) The window to use to setup the renderer for the first time
	 * \return The renderer singleton
	 */
	_NODISCARD __forceinline static Renderer& Get();
	/** function to explicitly initialize the singleton */
	__forceinline static Renderer& Create();
	/** Cleanup the renderer to make sure the engine exit properly */
	static void Shutdown();

public:
	/** Prepare a new frame to be renderer */
	void PrepareNewFrame();
	/** Render a new frame on the window */
	void RenderNewFrame();

	/** Called every frame */
	void Tick();

	__forceinline bool IsWindowClosed() const { return (glfwWindowShouldClose(s_Window) == GLFW_TRUE); }
	__forceinline static GLFWwindow* GetWindow() { return s_Window; }

public:
	__forceinline const VulkanSwapChainFrame& GetCurrentFrame() const { return m_VkSwapChain.GetFrame(m_CurrentFrameIndex); }
	__forceinline uint8_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

private:
	/** Create, initialize and setup the vulkan instance */
	void InitVulkanInstance();
	/** Create the GLFW window vulkan surface */
	void InitSurfaceKHR(GLFWwindow* window);
	/** Create, initialize and setup the vulkan device */
	void InitVulkanDevice();
	/** Create, initialize and setup the vulkan swap chain */
	void InitSwapChain();

	/** Record a memory barrier that will put the image in mode that allow shader to write onto it */
	void ShaderWriteBarrier(const vk::CommandBuffer& cmdBuffer, const vk::Image& image, vk::AccessFlagBits previousAccessFlag, vk::ImageLayout previousImageLayout);
	/** Record a memory barrier that will put the image in mode that all the image to be drawn on screen */
	void PresentBarrier(const vk::CommandBuffer cmdBuffer, const vk::Image& image, vk::AccessFlagBits previousAccessFlag, vk::ImageLayout previousImageLayout);
	/** Record the TraceRayKHR function with the shader binded to it */
	void TraceRays(const vk::CommandBuffer& cmdBuffer);

private:
	static GLFWwindow* s_Window;
	static Renderer* s_Instance;

	VulkanInstanceHandler m_VkInstance;
	VulkanDeviceHandler m_VkDevice;
	vk::CommandPool m_CommandPool;
	VulkanSwapChainHandler m_VkSwapChain;
	vk::SurfaceKHR m_Surface;
	VulkanRayTracingPipeline m_Pipeline;
	VulkanAccelerationStructure m_AccelerationStructure;
	VulkanShaderBindingTable m_ShaderBindingTable;
	vk::PipelineCache m_PipelineCache;

	vk::DispatchLoaderDynamic m_Dldi;

	uint8_t m_CurrentFrameIndex;

	vk::Image m_RayTracingImage;
	vk::DeviceMemory m_RayTracingImageMemory;
	vk::ImageView m_RayTracingImageView;

#ifdef WITH_EDITOR
	friend class UI;
#endif // WITH_EDITOR
};
