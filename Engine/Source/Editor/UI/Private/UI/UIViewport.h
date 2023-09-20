#pragma once

#include "UI.h"
#include "SceneRenderer.h"
#include "UISwapchain.h"
#include "UIImage.h"

#include <vulkan/vulkan.hpp>

class UIViewport : public UI
{

public:
	UIViewport(const UISwapchain& swapchain);
	~UIViewport();

protected:
	void Tick(float deltaTime) override;
	void Draw() override;

protected:
	void ResizeViewportImage(uint32_t width, uint32_t height);

protected:
	const UISwapchain& m_Swapchain;

	uint32_t m_Width;
	uint32_t m_Height;

	SceneRenderer m_SceneRenderer;

	std::vector<UIImage> m_ViewportImages;
	vk::Sampler m_Sampler;
};
