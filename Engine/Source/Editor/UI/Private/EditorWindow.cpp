#include "EditorWindow.h"
#include "VulkanContext.h"
#include "UIGlobals.h"
#include "ImGuiBackends.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

EditorWindow::EditorWindow(AxisSize width, AxisSize height, const char* title)
	: Window(width, height, title)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_WindowPtr = glfwCreateWindow(width, height, title, nullptr, nullptr);

	// Get GLFW vulkan extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
		VulkanContext::Get().AddInstanceExtension(glfwExtensions[i]);

	// Add swapchain extension
	VulkanContext::Get().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// Create the vulkan instance in 1.3.224
	// It's a static version, because I want to verify that the everything works before upgrading
	if (VulkanContext::Get().CreateInstance(1, 3, 224) == false)
		return;

	// Get window surface
	vk::SurfaceKHR surface;
	if (glfwCreateWindowSurface(VulkanContext::GetInstance(), m_WindowPtr, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)) != VK_SUCCESS)
		return;

	if (VulkanContext::Get().CreateDevice(surface) == false)
		return;

	/* DESCRIPTOR POOL */

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
	VulkanContext::GetDevice().createDescriptorPool(&pool_info, nullptr, &m_DescriptorPool);

	/* RENDER PASS */

	vk::SurfaceFormatKHR renderPassFormat = UISwapchain::SelectSurfaceFormat(surface);
	UI_LOG(Warning, "Selected surface format: {:s} ({:s})", vk::to_string(renderPassFormat.format), vk::to_string(renderPassFormat.colorSpace));

	vk::AttachmentDescription colorAttachment(
		vk::AttachmentDescriptionFlags(),
		renderPassFormat.format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	);

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::SubpassDescription subpass(
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		0, nullptr,
		1, &colorAttachmentRef,
		nullptr,
		nullptr,
		0, nullptr
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlags(),
		vk::AccessFlagBits::eColorAttachmentWrite
	);

	vk::RenderPassCreateInfo renderPassInfo(
		vk::RenderPassCreateFlags(),
		1, &colorAttachment,
		1, &subpass,
		1, &dependency
	);
	VulkanContext::GetDevice().createRenderPass(&renderPassInfo, nullptr, &m_RenderPass);

	/* Create swapchain */

	m_Swapchain = UISwapchain(vk::PresentModeKHR::eMailbox, surface, m_Width, m_Height, m_RenderPass);

	/* INIT IMGUI */

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	SetupDefaultStyle();

	ImGui_ImplGlfw_InitForVulkan(m_WindowPtr, true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = VulkanContext::GetInstance();
	initInfo.Device = VulkanContext::GetDevice();
	initInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice();
	initInfo.QueueFamily = VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics;
	initInfo.Queue = VulkanContext::GetDevice().getQueue(
		initInfo.QueueFamily, 0
	);
	initInfo.Subpass = 0;
	initInfo.DescriptorPool = m_DescriptorPool;
	initInfo.MinImageCount = m_Swapchain.GetFrameCount();
	initInfo.ImageCount = m_Swapchain.GetFrameCount();
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = [](VkResult error)
		{
			if (error != VK_SUCCESS)
				UI_LOG(Fatal, "ImGui_ImplVulkan_Init failed with error: {:s}", vk::to_string((vk::Result)error));
		};
	ImGui_ImplVulkan_Init(&initInfo, m_RenderPass);

	VulkanContext::GetDevice().SubmitOneTimeCommandBuffer(
		VulkanContext::GetDevice().GetQueueFamilyIndicies().Graphics,
		[](vk::CommandBuffer commandBuffer)
		{
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		}
	);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

EditorWindow::~EditorWindow()
{
	VulkanContext::GetDevice().waitIdle();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	VulkanContext::GetDevice().destroyDescriptorPool(m_DescriptorPool);
	VulkanContext::GetDevice().destroyRenderPass(m_RenderPass);

	m_Swapchain.Destroy();

	glfwDestroyWindow(m_WindowPtr);
}

bool EditorWindow::IsClosed() const
{
	return glfwWindowShouldClose(m_WindowPtr);
}

void EditorWindow::SetSize(AxisSize width, AxisSize height)
{
	m_Width = width;
	m_Height = height;
	glfwSetWindowSize(m_WindowPtr, width, height);
	m_HasBeenResized = true;
}

void EditorWindow::Tick(float deltaTime)
{
	glfwPollEvents();

	// Get window size if it has been resized
	int width, height;
	glfwGetWindowSize(m_WindowPtr, &width, &height);

	if (width != m_Width || height != m_Height)
	{
		m_Width = width;
		m_Height = height;
		m_HasBeenResized = true;
	}

	if (m_HasBeenResized)
	{
		UI_LOG(Verbose, "Window has been resized to {:d}x{:d}", m_Width, m_Height);
		m_Swapchain.Resize(m_Width, m_Height);
		m_HasBeenResized = false;
	}
}

void EditorWindow::Draw()
{
	const VulkanSwapchainFrame& frame = m_Swapchain.AcquireNextFrame();

	frame.Begin();

	// TODO: Ask to draw the scene on the frame
	static bool show_demo_window = true;
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Viewport");
	ImGui::Text("Hello, world!");
	ImGui::End();

	frame.End();

	m_Swapchain.PresentFrame();
}

void EditorWindow::OnTitleUpdate()
{
	glfwSetWindowTitle(m_WindowPtr, m_Title);
}

void EditorWindow::SetupDefaultStyle()
{
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
