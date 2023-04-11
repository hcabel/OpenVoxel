#pragma once

#include "RendererModule.h"
#include "Vulkan/VulkanInstanceHandler.h"
#include "Vulkan/VulkanDeviceHandler.h"
#include "Vulkan/VulkanSwapChainHandler.h"
#include "Vulkan/VulkanAccelerationStructure.h"

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
	_NODISCARD static Renderer* Get();
	/** function to explicitly initialize the singleton */
	static Renderer* Initialize(GLFWwindow* window);
	/** Cleanup the renderer to make sure the engine exit properly */
	static void Shutdown();

public:
	/** Prepare a new frame to be renderer */
	void PrepareNewFrame();
	/** Render a new frame on the window */
	void RenderNewFrame();

private:
	/** Initialize, setup, and create the vulkan instance */
	void InitVulkanInstance();
	/** Create the GLFW window vulkan surface */
	void InitSurfaceKHR(GLFWwindow* window);
	/** Initialize, setup, and create the vulkan device */
	void InitVulkanDevice();

	void InitSwapChain();

private:
	static Renderer* s_Instance;

	VulkanInstanceHandler m_VkInstance;
	VulkanDeviceHandler m_VkDevice;
	vk::CommandPool m_CommandPool;
	VulkanSwapChainHandler m_VkSwapChain;
	vk::SurfaceKHR m_Surface;
	VulkanAccelerationStructure m_AccelerationStructure;

	uint8_t m_CurrentFrameIndex;
};