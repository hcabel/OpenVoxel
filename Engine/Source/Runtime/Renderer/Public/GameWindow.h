#pragma once

#include "Renderer_API.h"
#include "Window.h"
#include "VulkanSwapchain.h"

class GLFWwindow;
class VulkanSwapchain;

/**
 * This class is a window implementation using GLFW.
 */
class RENDERER_API GameWindow final : public Window
{
public:
	GameWindow(AxisSize width, AxisSize height, const char* title);
	~GameWindow();

public:
	__forceinline bool IsClosed() const override;

	void SetSize(AxisSize width, AxisSize height) override;

private:
	void Tick(float deltaTime) override;
	void Draw() override;

	void OnTitleUpdate() override;

private:
	/* A ptr to the actual GLFW window */
	GLFWwindow* m_WindowPtr;
	VulkanSwapchain m_Swapchain;

};
