#include "UIViewport.h"
#include "EditorWindow.h"
#include "VulkanContext.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuiBackends.h>

UIViewport::UIViewport(const UISwapchain& swapchain)
	: UI(),
	m_Swapchain(swapchain),
	m_Width(0),
	m_Height(0)
{
	m_SceneRenderer = SceneRenderer::Create(swapchain.GetFrameCount());
}

UIViewport::~UIViewport()
{
	m_ViewportImages.clear();
}

void UIViewport::Tick(float deltaTime)
{
}

void UIViewport::Draw()
{
	ImGuiWindowClass windowClass;
	windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::SetNextWindowClass(&windowClass);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove))
	{
		const ImVec2 size = ImGui::GetContentRegionAvail();
		if (size.x > 0 && size.y > 0)
		{
			if ((int)size.x != m_Width || (int)size.y != m_Height)
			{
				uint32_t width = (uint32_t)size.x;
				uint32_t height = (uint32_t)size.y;

				ResizeViewportImage(size.x, size.y);

				m_Width = size.x;
				m_Height = size.y;
			}

			auto cmdBuffer = m_Swapchain.GetCurrentFrame().GetCommandBuffer();

			cmdBuffer.pipelineBarrier( // Make the frame ready for rendering
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::DependencyFlagBits::eByRegion,
				{},
				{},
				vk::ImageMemoryBarrier(
					vk::AccessFlagBits::eMemoryRead,
					vk::AccessFlagBits::eShaderWrite,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					m_ViewportImages[m_Swapchain.GetCurrentFrameIndex()].GetImage(),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
				)
			);

			m_SceneRenderer.RenderScene(
				SceneRenderer::RenderSceneInfo(
					m_Swapchain.GetCurrentFrameIndex(),
					m_Width,
					m_Height,
					m_ViewportImages[m_Swapchain.GetCurrentFrameIndex()].GetView(),
					cmdBuffer
				)
			);

			ImTextureID textureId = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(m_ViewportImages[m_Swapchain.GetCurrentFrameIndex()]));
			ImGui::Image(textureId, ImVec2(m_Width, m_Height));
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void UIViewport::ResizeViewportImage(uint32_t width, uint32_t height)
{
	uint32_t i = 0;

	// Resize current images
	while (i < m_ViewportImages.size())
	{
		m_ViewportImages[i].Resize(width, height);
		i++;
	}

	// If still not enough images, create new ones
	uint32_t frameCount = m_Swapchain.GetFrameCount();
	if (i < frameCount)
	{
		vk::ImageCreateInfo imageCreateInfo(
			vk::ImageCreateFlags(),
			vk::ImageType::e2D,
			m_Swapchain.GetFrameFormat(),
			vk::Extent3D(width, height, 1),
			1,
			1,
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined
		);

		vk::ImageViewCreateInfo viewCreateInfo(
			vk::ImageViewCreateFlags(),
			VK_NULL_HANDLE, // Will be set by UIImage
			vk::ImageViewType::e2D,
			m_Swapchain.GetFrameFormat(),
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
		);

		vk::SamplerCreateInfo samplerCreateInfo(
			vk::SamplerCreateFlags(),
			vk::Filter::eLinear,
			vk::Filter::eLinear,
			vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat,
			0.0f,
			VK_TRUE,
			16.0f,
			VK_FALSE,
			vk::CompareOp::eAlways,
			0.0f,
			0.0f,
			vk::BorderColor::eIntOpaqueBlack,
			VK_FALSE
		);

		m_ViewportImages.reserve(frameCount);
		while (i < frameCount)
		{
			m_ViewportImages.emplace_back(UIImage(imageCreateInfo, viewCreateInfo, samplerCreateInfo));

			i++;
		}
	}
}
