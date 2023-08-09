#pragma once

#include "UI_API.h"

class VulkanSwapChainFrame;

#include <vulkan/vulkan.hpp>

/**
 * This class is in charge of rendering the whole editor UI.
 */
class UI_API UI
{
public:
	__forceinline static UI& Get()
	{
		static UI instance;
		return (instance);
	}

public:
	void PrepareNewFrame();

	void RenderNewFrame();

	void ShutDown(); // will be called by UIModule
private:
	void Init(); // will be called by UIModule
	void SetupDefaultStyle();
	void RecordFrameCmdBuffer(const VulkanSwapChainFrame& frame);

	void CreateRenderPass();
	void CreateFrameBuffers();
	void UploadUIFont();

private:
	vk::RenderPass m_RenderPass;
	std::vector<vk::Framebuffer> m_FrameBuffers;
	vk::DescriptorPool m_DescriptorPool;

	friend class UIModule;
};
