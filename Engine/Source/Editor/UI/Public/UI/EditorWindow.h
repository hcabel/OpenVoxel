#pragma once

#include "UI_API.h"
#include "Window.h"
#include "UISwapchain.h"
#include "UI.h"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

class UI_API EditorWindow : public Window, public UI
{
public:
	EditorWindow(AxisSize width, AxisSize height, const char* title);
	~EditorWindow();

public:
	__forceinline bool IsClosed() const override;

	void SetSize(AxisSize width, AxisSize height) override;

protected:
	void Tick(float deltaTime) override;
	void Draw() override;
	void OnTitleUpdate() override;

protected:
	void SetupDefaultStyle();

protected:
	UISwapchain m_Swapchain;
	vk::DescriptorPool m_DescriptorPool;
	vk::RenderPass m_RenderPass;

	GLFWwindow* m_WindowPtr;
};
