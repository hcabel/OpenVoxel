#pragma once

#include "CoreModule.h"
#include "Logging/LoggingMacros.h"
#include "Vulkan/VulkanImpl.h"
#include "Vulkan/VulkanSwapchain.h"
#include "Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <optional>

DECLARE_LOG_CATEGORY(GlfwLog);

/**
 * Manage GLFW window.
 */
class CORE_API MainWindow final
{

public:
	MainWindow(uint16_t width, uint16_t height, const char *title);
	~MainWindow();

	/** Check if the glfw should close */
	bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }

	void NewFrame();

	operator GLFWwindow*() { return (m_Window); }
	GLFWwindow* GetWindows() { return (m_Window); }

private:
	std::optional<GLFWwindow*> CreateWindow(uint16_t width, uint16_t height, const char *title) const noexcept;

#pragma region Properties
private:
	GLFWwindow *m_Window;

	vk::Instance m_VkInstance;
#if OV_DEBUG
	Vulkan::DebugMessenger m_DebugMessenger;
#endif
	vk::SurfaceKHR m_Surface;
	Vulkan::DeviceBundle m_Device;
	vk::CommandPool m_CommandPool;
	std::unique_ptr<VulkanSwapchain> m_Swapchain;

	Renderer *m_Renderer;

#pragma endregion
};
