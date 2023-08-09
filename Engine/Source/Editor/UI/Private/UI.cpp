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
	SetupDefaultStyle();

	ImGui_ImplGlfw_InitForVulkan(Renderer::GetWindow(), true);
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

void UI::SetupDefaultStyle()
{
	// OpenVoxel Copy style from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	/* DESIGN */

	// "Future Dark" by rewrking, published on ImThemes:
	// https://github.com/Patitotective/ImThemes
	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(12.0f, 12.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(6.0f, 6.0f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(12.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
	style.CellPadding = ImVec2(12.0f, 6.0f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 12.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	// My part :D
	style.DisabledAlpha = 0.2f;
	style.WindowRounding = 6.0f;
	style.ChildRounding = 6.0f;
	style.PopupRounding = 6.0f;
	style.FramePadding = ImVec2(6.0f, 3.5f);
	style.FrameRounding = 5.0f;
	style.ItemSpacing = ImVec2(5.0f, 8.0f);
	style.ItemInnerSpacing = ImVec2(10.0f, 10.0f);
	style.CellPadding = ImVec2(6.0f, 6.0f);
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 4.0f;
	style.GrabMinSize = 15.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 5.0f;

	/* COLORS */

	// "Future Dark" by rewrking, published on ImThemes:
	// https://github.com/Patitotective/ImThemes
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);

	// My part :D (This is just a global default theme, I'll probably abstract some components to have their own color behavior (like button and slider for example) 
	constexpr auto colorFromBytes = [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return ImVec4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }; };
	constexpr auto colorWithAnotherAlpha = [](const ImVec4& color, uint8_t a) { return ImVec4{ color.x, color.y, color.z, a / 255.0f }; };

	// Colorful highlight
	constexpr ImVec4 blueBright = colorFromBytes(71, 203, 255, 255);
	constexpr ImVec4 blue = colorFromBytes(56, 161, 200, 255);
	constexpr ImVec4 blueFaded = colorWithAnotherAlpha(blue, 100);
	constexpr ImVec4 blueDark = colorFromBytes(43, 55, 153, 255);

	constexpr ImVec4 peachBright = colorFromBytes(255, 188, 133, 255);
	constexpr ImVec4 peach = colorFromBytes(232, 171, 121, 255);
	constexpr ImVec4 peachDark = colorFromBytes(150, 110, 78, 255);

	constexpr ImVec4 green = colorFromBytes(128, 242, 107, 255);

	constexpr ImVec4 red = colorFromBytes(251, 117, 114, 255);

	constexpr ImVec4 purpleBright = colorFromBytes(210, 155, 255, 255);
	constexpr ImVec4 purple = colorFromBytes(193, 117, 255, 255);

	style.Colors[ImGuiCol_TextSelectedBg] = blueDark;

	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.16f, 0.19f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = peach;

	style.Colors[ImGuiCol_ScrollbarGrabHovered] = peach;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = peachBright;
	
	style.Colors[ImGuiCol_CheckMark] = green;

	style.Colors[ImGuiCol_SliderGrab] = peach;
	style.Colors[ImGuiCol_SliderGrabActive] = peachBright;

	style.Colors[ImGuiCol_ButtonHovered] = peach;
	style.Colors[ImGuiCol_ButtonActive] = peachBright;

	style.Colors[ImGuiCol_HeaderHovered] = peach;
	style.Colors[ImGuiCol_HeaderActive] = peachBright;

	style.Colors[ImGuiCol_ResizeGrip] = colorWithAnotherAlpha(peachDark, 50);
	style.Colors[ImGuiCol_ResizeGripHovered] = peach;
	style.Colors[ImGuiCol_ResizeGripActive] = peachDark;

	style.Colors[ImGuiCol_Tab] = blueFaded;
	style.Colors[ImGuiCol_TabHovered] = blueBright;
	style.Colors[ImGuiCol_TabActive] = blue;
	style.Colors[ImGuiCol_TabUnfocused] = blueFaded;
	style.Colors[ImGuiCol_TabUnfocusedActive] = blueFaded;

	style.Colors[ImGuiCol_PlotLinesHovered] = red;
	style.Colors[ImGuiCol_PlotHistogram] = purple;
	style.Colors[ImGuiCol_PlotHistogramHovered] = purpleBright;

	// I haven't seen these in the UI yet, but this are default colors
	style.Colors[ImGuiCol_DragDropTarget] = peach;
	style.Colors[ImGuiCol_NavHighlight] = peach;
	style.Colors[ImGuiCol_NavWindowingHighlight] = peach;

	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.078f, 0.086f, 0.10f, 0.58f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.078f, 0.086f, 0.10f, 0.58f);
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
