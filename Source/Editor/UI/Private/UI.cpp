#include "UI.h"
#include "Renderer.h"
#include "CoreGlobals.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_glfw.cpp>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_vulkan.cpp>

void UI::Init()
{
	VulkanDeviceHandler& vulkanDevice = Renderer::Get().m_VkDevice;

	vk::DescriptorPoolSize pool_sizes[] =
	{
		{ vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 }
	};

	vk::DescriptorPoolCreateInfo pool_info(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		1000,
		std::size(pool_sizes),
		pool_sizes
	);
	vulkanDevice.Raw().createDescriptorPool(&pool_info, nullptr, &m_DescriptorPool);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(RendererModule::GetWindow(), true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = Renderer::Get().m_VkInstance;
	initInfo.Device = vulkanDevice;
	initInfo.PhysicalDevice = vulkanDevice.GetPhysicalDevice();
	initInfo.Queue = vulkanDevice.GetQueue(VulkanQueueType::Graphic);
	initInfo.QueueFamily = vulkanDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex;
	auto& pipeline = Renderer::Get().m_Pipeline;
	initInfo.PipelineCache = Renderer::Get().m_PipelineCache;
	initInfo.Subpass = 0;
	initInfo.DescriptorPool = m_DescriptorPool;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = 2;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = [](VkResult error)
		{
			if (error != VK_SUCCESS)
				OV_LOG(LogTemp, Fatal, "ImGui_ImplVulkan_Init failed with error: {:s}", vk::to_string((vk::Result)error));
		};

	CreateRenderPass();
	CreateFrameBuffers();

	ImGui_ImplVulkan_Init(&initInfo, m_RenderPass);

	UploadUIFont();
}

void UI::ShutDown()
{
	VulkanDeviceHandler& vulkanDevice = Renderer::Get().m_VkDevice;
	vulkanDevice.Raw().waitIdle();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vulkanDevice.Raw().destroyRenderPass(m_RenderPass);
	for (auto& frameBuffer : m_FrameBuffers)
	{
		vulkanDevice.Raw().destroyFramebuffer(frameBuffer);
	}

	vulkanDevice.Raw().destroyDescriptorPool(m_DescriptorPool);
}

void UI::PrepareNewFrame()
{
	Renderer::Get().PrepareNewFrame();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UI::RenderNewFrame()
{

	static bool show_demo_window = true;
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Another window!");
	ImGui::Text("This is some useful text.");
	ImGui::End();

	ImGui::Render();

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	const VulkanSwapChainFrame frame = Renderer::Get().GetCurrentFrame();

	RecordFrameCmdBuffer(frame);

	Renderer::Get().m_VkSwapChain.SubmitWork(Renderer::Get().GetCurrentFrameIndex());
	Renderer::Get().m_VkSwapChain.PresentFrame(Renderer::Get().GetCurrentFrameIndex());
}

void UI::RecordFrameCmdBuffer(const VulkanSwapChainFrame& frame)
{
	VulkanDeviceHandler& vulkanDevice = Renderer::Get().m_VkDevice;

	frame.CommandBuffer.reset();
	frame.CommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	frame.CommandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, Renderer::Get().m_Pipeline);

	// Bind the descriptor set (allow shader to access the data stored in the descriptor set)
	frame.CommandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR,
		Renderer::Get().m_Pipeline.GetPipelineLayout(),
		0,
		1, &frame.DescriptorSet,
		0,
		nullptr
	);

	vk::ImageMemoryBarrier shaderWriteBarrier(
		vk::AccessFlagBits::eMemoryRead,
		vk::AccessFlagBits::eShaderWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eGeneral,
		vulkanDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		vulkanDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		frame.Image,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);
	frame.CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &shaderWriteBarrier
	);
	Renderer::Get().TraceRays(frame.CommandBuffer);

	vk::ImageMemoryBarrier swapchainPresentMemoryBarrier2(
		vk::AccessFlagBits::eShaderWrite,
		vk::AccessFlagBits::eColorAttachmentWrite,
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eColorAttachmentOptimal,
		vulkanDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		vulkanDevice.GetQueue(VulkanQueueType::Graphic).FamilyIndex,
		frame.Image,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		)
	);
	frame.CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &swapchainPresentMemoryBarrier2
	);

	vk::RenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.framebuffer = m_FrameBuffers[Renderer::Get().GetCurrentFrameIndex()];
	renderPassBeginInfo.renderArea.extent = Renderer::Get().m_VkSwapChain.GetExtent();

	frame.CommandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frame.CommandBuffer, nullptr);
	frame.CommandBuffer.endRenderPass();

	// Renderer::Get().PresentBarrier(frame.CommandBuffer, frame.Image, vk::AccessFlagBits::eColorAttachmentWrite, vk::ImageLayout::eColorAttachmentOptimal);

	frame.CommandBuffer.end();
}

void UI::CreateRenderPass()
{
	VulkanDeviceHandler& vulkanDevice = Renderer::Get().m_VkDevice;

	std::array<vk::AttachmentDescription, 1> attachments = {};
	attachments[0].format = Renderer::Get().m_VkSwapChain.GetFormat();
	attachments[0].samples = vk::SampleCountFlagBits::e1;
	attachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;

	vk::SubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
	subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo renderPassCreateInfo(
		vk::RenderPassCreateFlags(),
		static_cast<uint32_t>(attachments.size()), attachments.data(),
		1, &subpass,
		1, &subpassDependency
	);

	vulkanDevice.Raw().createRenderPass(&renderPassCreateInfo, nullptr, &m_RenderPass);
}

void UI::CreateFrameBuffers()
{
	for (auto& frame : Renderer::Get().m_VkSwapChain.GetFrames())
	{
		vk::FramebufferCreateInfo framebufferCreateInfo(
			vk::FramebufferCreateFlags(),
			m_RenderPass,
			1, &frame.ImageView,
			Renderer::Get().m_VkSwapChain.GetExtent().width,
			Renderer::Get().m_VkSwapChain.GetExtent().height,
			1
		);

		m_FrameBuffers.push_back(
			Renderer::Get().m_VkDevice.Raw().createFramebuffer(framebufferCreateInfo)
		);
	}
}

void UI::UploadUIFont()
{
	VulkanDeviceHandler& vulkanDevice = Renderer::Get().m_VkDevice;

	vk::CommandBuffer cmdBuffer = vulkanDevice.Raw().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(
			Renderer::Get().m_CommandPool,
			vk::CommandBufferLevel::ePrimary,
			1
		)
	)[0];

	cmdBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);

	cmdBuffer.end();

	vk::Fence fence = vulkanDevice.Raw().createFence(vk::FenceCreateInfo());

	vk::SubmitInfo submitInfo(
		0, nullptr, nullptr,
		1, &cmdBuffer,
		0, nullptr
	);
	vulkanDevice.GetQueue(VulkanQueueType::Graphic).submit(submitInfo, fence);

	vulkanDevice.Raw().waitForFences(fence, VK_TRUE, UINT64_MAX);
	vulkanDevice.Raw().destroyFence(fence);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}
