#include "pch.h"
#include "Editor.h"

#include "Core/Engine.h"
#include "Utilities/Timer.h"
#include "Platform/PlatformEntry.h"

#include "Vulkan/GraphhicsPipeline.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Shader.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/VulkanVertexBuffer.h"
#include "Vulkan/VulkanContext.h"


#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "Vulkan/VulkanRenderer.h"
#include "Renderer/RuntimeRenderer.h"
#include "Renderer/RenderAPI.h"
#include "Vulkan/VulkanFramebuffer.h"
#include "Map/World.h"
#include "ImguiUtil.h"
#include "Core/MemoryTracker.h"
#include "Utilities/Colors.h"
#include "Utilities/Ray.h"
#include "Renderer/OrthographicCamera.h"
#include "Event/Input.h"
#include "glm/vector_relational.hpp"

namespace PAL
{
	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
	static SharedPtr<VulkanFramebuffer> s_ImGuiFramebuffer;
	static VkDescriptorPool s_DescriptorPool = VK_NULL_HANDLE;

	Editor::Editor()
		: m_HierarchyPanel(m_CurrentWorld)
	{
	}

	Editor::~Editor()
	{
	}

	void Editor::Init()
	{
		m_Swapchain = Engine::Get()->GetWindow()->GetSwapChain();

		RuntimeRendererSpecification spec = {};
		spec.TargetAPI = RenderAPITarget::Vulkan;
		spec.ViewportSize = m_ViewportSize;

		RenderpassSpecification& mainRenderpass = spec.ApiSpec.MainRenderpassSpec;
		mainRenderpass.ColorFormat = m_Swapchain->GetSwapChainImageFormat();
		mainRenderpass.Target = PresentTarget::CustomViewport;
		mainRenderpass.UseDepth = false;
		mainRenderpass.DebugName = "Main Renderpass";

		FramebufferSpecification& mainFramebuffer = spec.ApiSpec.MainFrameBufferSpec;
		mainFramebuffer.ColorFormat = m_Swapchain->GetSwapChainImageFormat();
		mainFramebuffer.Width = Engine::Get()->GetWindow()->GetWindowWidth();
		mainFramebuffer.Height = Engine::Get()->GetWindow()->GetWindowHeight();
		mainFramebuffer.UseDepth = false;
		mainFramebuffer.Target = PresentTarget::CustomViewport;
		mainFramebuffer.DebugName = "Main Framebuffer";

		m_RuntimeRenderer = NewShared<RuntimeRenderer>();
		m_RuntimeRenderer->Init(spec);

		m_CurrentWorld = NewShared<World>();
		m_CurrentWorld->BeginPlay(); // move later

		InitImGui();
	}

	void Editor::OnUpdate(float deltaTime)
	{
		CORE_PROFILER_FUNC();

		auto& window = Engine::Get()->GetWindow();

		//m_RuntimeRenderer->ResizeFrameBuffer(window->GetWindowWidth(), window->GetWindowHeight());

		if (m_RuntimeRenderer->GetRuntimeRendererSpec().ApiSpec.MainFrameBufferSpec.Target == PresentTarget::CustomViewport)
		{
			// When viewport outside the window, it does not render !!
			m_RuntimeRenderer->ResizeFrameBuffer((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			//m_CurrentWorld->GetActiveCamera().As<OrthographicCamera>()->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
		else
		{
			m_CurrentWorld->GetActiveCamera().As<OrthographicCamera>()->SetViewportSize(window->GetWindowWidth(), window->GetWindowHeight());
		}

		m_RuntimeRenderer->StartFrame();

		m_CurrentWorld->OnUpdate(deltaTime);
		m_CurrentWorld->OnRender(deltaTime);

		m_RuntimeRenderer->FlushDrawCommands();

		// Imgui UI on top 
		{
			CORE_PROFILER_SCOPE("RenderImGui");
			BeginFrame();
			RenderImGui();
			EndFrame();
		}

		m_RuntimeRenderer->EndFrame();
	}

	void Editor::OnShutdown()
	{
		m_RuntimeRenderer->WaitForIdle();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(VulkanContext::GetVulkanDevice()->GetVkDevice(), s_DescriptorPool, nullptr);
		vkDestroyRenderPass(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_ImGuiRenderPass, nullptr);
		s_ImGuiFramebuffer->Shutdown();

		m_RuntimeRenderer->Shutdown();
	}

	void Editor::SetupVulkan()
	{
		const auto& swapchain = Engine::Get()->GetWindow()->GetSwapChain();

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::GetVulkanDevice()->GetVkDevice(), &pool_info, nullptr, &s_DescriptorPool));

		ImGui_ImplGlfw_InitForVulkan(*Engine::Get()->GetWindow(), true);

		// RenderPass
		{
			VkAttachmentDescription attachments[1];
			attachments[0].format = Engine::Get()->GetWindow()->GetSwapChain()->GetSwapChainImageFormat();
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

			// if DONOT_CARE the ui will be rendered on top of the prev one (the prev one is responsible for clearing)
			// @TODO: Create a render pass just for clearing 

			if (m_RuntimeRenderer->GetRuntimeRendererSpec().ApiSpec.MainRenderpassSpec.Target == PresentTarget::CustomViewport)
				attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			else
				attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Use this if we render the main pass to the window with imgui 

			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			attachments[0].flags = 0;

			VkAttachmentReference color_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.flags = 0;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = NULL;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_reference;
			subpass.pResolveAttachments = NULL;
			subpass.pDepthStencilAttachment = NULL;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = NULL;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo rp_info = {};
			rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			rp_info.pNext = NULL;
			rp_info.attachmentCount = 1;
			rp_info.pAttachments = attachments;
			rp_info.subpassCount = 1;
			rp_info.pSubpasses = &subpass;
			rp_info.dependencyCount = 1;
			rp_info.pDependencies = &dependency;

			VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::GetVulkanDevice()->GetVkDevice(), &rp_info, nullptr, &m_ImGuiRenderPass));
		}

		FramebufferSpecification imguiSpec = {};
		imguiSpec.ColorFormat = swapchain->GetSwapChainImageFormat();
		imguiSpec.DepthFormat = VK_FORMAT_UNDEFINED;
		imguiSpec.Width = swapchain->GetSwapChainExtent().width;
		imguiSpec.Height = swapchain->GetSwapChainExtent().height;
		imguiSpec.UseDepth = false;
		imguiSpec.Target = PresentTarget::Swapchain;
		imguiSpec.DebugName = "imgui Framebuffer";

		s_ImGuiFramebuffer = NewShared<VulkanFramebuffer>(m_ImGuiRenderPass, imguiSpec);

		// Setup Platform/Renderer backends
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanContext::GetVulkanInstance();
		init_info.PhysicalDevice = VulkanContext::GetVulkanDevice()->GetPhysicalDevice()->GetVkPhysicalDevice();
		init_info.Device = VulkanContext::GetVulkanDevice()->GetVkDevice();
		init_info.QueueFamily = VulkanContext::GetVulkanDevice()->GetPhysicalDevice()->FindQueueFamilies().graphicsFamily.value();
		init_info.Queue = VulkanContext::GetVulkanDevice()->GetVkGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = s_DescriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = (uint32_t)swapchain->GetSwapChainImages().size();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;

		ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass);

		// ImGui FrameBuffer

		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();
		s_ImGuiCommandBuffers.resize(framesInFlight);
		for (int i = 0; i < framesInFlight; i++)
		{
			s_ImGuiCommandBuffers[i] = VulkanContext::GetVulkanDevice()->GetMainCommandBuffer()->CreateSecondaryCommandBuffer();
		}
	}

	void Editor::InitImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup ImGui style
		ImGui::StyleColorsDark();
		SetStyleDark();

		SetupVulkan();

		// Upload Font
		{
			float fontSize = 20.0f;

			auto boldFont = io.Fonts->AddFontFromFileTTF(PROJECT_ROOT "/Source/Engine/res/fonts/opensans/OpenSans-Bold.ttf", fontSize);
			auto regularFont = io.Fonts->AddFontFromFileTTF(PROJECT_ROOT "/Source/Engine/res/fonts/opensans/OpenSans-Regular.ttf", fontSize);
			auto lightFont = io.Fonts->AddFontFromFileTTF(PROJECT_ROOT "/Source/Engine/res/fonts/opensans/OpenSans-Light.ttf", fontSize);
			auto boldItalicFont = io.Fonts->AddFontFromFileTTF(PROJECT_ROOT "/Source/Engine/res/fonts/opensans/OpenSans-BoldItalic.ttf", fontSize);
			auto regularItalicFont = io.Fonts->AddFontFromFileTTF(PROJECT_ROOT "/Source/Engine/res/fonts/opensans/OpenSans-Italic.ttf", fontSize);
			auto lightItalicFont = io.Fonts->AddFontFromFileTTF(PROJECT_ROOT "/Source/Engine/res/fonts/opensans/OpenSans-LightItalic.ttf", fontSize);

			io.FontDefault = regularFont;

			//m_ImGuiFonts.emplace(ImGuiFonts::Regular, regularFont);
			//m_ImGuiFonts.emplace(ImGuiFonts::Bold, boldFont);
			//m_ImGuiFonts.emplace(ImGuiFonts::Light, lightFont);
			//m_ImGuiFonts.emplace(ImGuiFonts::BoldItalic, boldItalicFont);
			//m_ImGuiFonts.emplace(ImGuiFonts::RegularItalic, regularItalicFont);
			//m_ImGuiFonts.emplace(ImGuiFonts::LightItalic, lightItalicFont);

			ImGui_ImplVulkan_CreateFontsTexture();
		}

		m_HierarchyPanel.SetWorld(m_CurrentWorld);
	}

	void Editor::SetStyleLight()
	{
		// General
		auto* style = &ImGui::GetStyle();
		style->FrameRounding = 2.0f;
		style->WindowPadding = ImVec2(4.0f, 3.0f);
		style->FramePadding = ImVec2(4.0f, 4.0f);
		style->ItemSpacing = ImVec2(4.0f, 3.0f);
		style->IndentSpacing = 12;
		style->ScrollbarSize = 12;
		style->GrabMinSize = 9;

		// Sizes
		style->WindowBorderSize = 0.0f;
		style->ChildBorderSize = 0.0f;
		style->PopupBorderSize = 0.0f;
		style->FrameBorderSize = 0.0f;
		style->TabBorderSize = 0.0f;


		style->WindowRounding = 0.0f;
		style->ChildRounding = 0.0f;
		style->FrameRounding = 0.0f;
		style->PopupRounding = 0.0f;
		style->GrabRounding = 2.0f;
		style->ScrollbarRounding = 12.0f;
		style->TabRounding = 0.0f;

		auto& colors = ImGui::GetStyle().Colors;

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

		// Backgrounds
		colors[ImGuiCol_WindowBg] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);

		// Borders
		colors[ImGuiCol_Border] = ImVec4(0.89f, 0.89f, 0.89f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Frames
		colors[ImGuiCol_FrameBg] = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.69f, 0.07f, 0.69f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

		// Menus
		colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.00f, 0.69f, 0.07f, 0.69f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Sliders & CheckMark
		colors[ImGuiCol_CheckMark] = ImVec4(0.01f, 0.01f, 0.01f, 0.63f);
		colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.69f, 0.07f, 0.69f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.69f, 0.07f, 0.69f);
		colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Headers 
		colors[ImGuiCol_Header] = ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Separators
		colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Resize Grips
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.18f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 0.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);

		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.82f, 0.46f, 0.69f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

		// Plot
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

		// Miscellaneous
		colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.69f, 0.07f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void Editor::SetStyleDark()
	{
		// General Style Settings
		auto& style = ImGui::GetStyle();

		// Sizing and Spacing
		style.WindowPadding = ImVec2(4.0f, 3.0f);
		style.FramePadding = ImVec2(4.0f, 4.0f);
		style.ItemSpacing = ImVec2(4.0f, 3.0f);
		style.IndentSpacing = 12;
		style.ScrollbarSize = 12;
		style.GrabMinSize = 9;

		// Borders and Rounding
		style.FrameRounding = 2.0f;
		style.WindowBorderSize = 0.0f;
		style.ChildBorderSize = 0.0f;
		style.PopupBorderSize = 0.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;
		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.GrabRounding = 2.0f;
		style.ScrollbarRounding = 12.0f;
		style.TabRounding = 0.0f;

		// Color Definitions
		auto& colors = ImGui::GetStyle().Colors;
		// TODO: global Array of colors 

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

		// Backgrounds
		colors[ImGuiCol_WindowBg] = ImVec4(0.08235f, 0.08235f, 0.08235f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.196f, 0.196f, 0.196f, 1.00f);

		// Borders
		colors[ImGuiCol_Border] = ImVec4(0.101f, 0.101f, 0.101f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);

		// Frames
		colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.15f, 0.16f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);

		// Menus
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

		// Sliders & CheckMark
		colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

		// Buttons
		colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);

		// Headers
		colors[ImGuiCol_Header] = Colors::Light_Black;
		colors[ImGuiCol_HeaderHovered] = Colors::Light_Black;
		colors[ImGuiCol_HeaderActive] = Colors::Light_Black;

		// Separators
		colors[ImGuiCol_Separator] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);

		// Resize Grips
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.84f, 0.34f, 0.17f, 0.14f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab] = Colors::Light_Black;
		colors[ImGuiCol_TabHovered] = Colors::Light_Black;
		colors[ImGuiCol_TabActive] = Colors::Light_Black;
		colors[ImGuiCol_TabUnfocused] = Colors::Light_Black;
		colors[ImGuiCol_TabUnfocusedActive] = Colors::Light_Black;

		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

		// Plot
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

		// Miscellaneous
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);

	}

	void Editor::RenderImGui()
	{
		CORE_PROFILER_FUNC();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		//bool demoWindow = true;
		//ImGui::ShowDemoWindow(&demoWindow);

		RenderMenuBar();
		m_HierarchyPanel.ImGuiRender();
		RenderStatsPanel();
		RenderSettingsPanel();
		RenderViewport();

		ImGui::End();
	}

	void Editor::RenderMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					//NewScene();
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					//OpenScene();
				}

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					//SaveScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					//SaveSceneAs();
				}

				if (ImGui::MenuItem("Exit"))
				{
					Engine::Get()->Stop();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void Editor::RenderStatsPanel()
	{
		ImGui::Begin("Stats");

		RenderStats& stats = RuntimeRenderer::GetStats();
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.Quads);
		ImGui::Text("VertexBufferCount: %d", stats.VertexBufferCount);
		ImGui::Text("FrameTime : %.4fs", stats.FrameTime);
		ImGui::Text("FrameTime : %.4fms", stats.FrameTime_ms);
		ImGui::Text("FPS: %d", stats.FramesPerSecond);
		ImGui::Text("DrawCommandsQueue Usage: %d bytes", stats.DrawCommandsQueueUsage);

		ImGui::NewLine();
		ImGui::Text("MemoryUsage: %.5f mb", AllocationTracker::GetCurrentUsage());
		ImGui::Text("TotalAllocted: %.5f mb", AllocationTracker::GetTotalAllocated());
		ImGui::Text("TotalFreed: %.5f mb", AllocationTracker::GetTotalFreed());

		ImGui::NewLine();
		ImGui::Text("Editor Camera: %f, %f", m_CurrentWorld->GetActiveCamera()->GetPosition().x, m_CurrentWorld->GetActiveCamera()->GetPosition().y);

		ImGui::End();
	}

	void Editor::RenderSettingsPanel()
	{
		ImGui::Begin("Settings");
		auto& window = Engine::Get()->GetWindow();

		bool vsync = window->IsVsyncOn();
		if (ImGui::Checkbox("VSync", &vsync))
		{
			Engine::Get()->SetVSync(vsync);
		}

		static bool s_LightTheme = false;
		if (ImGui::Checkbox("Light Mode!", &s_LightTheme))
		{
			if (s_LightTheme)
			{
				ImGui::StyleColorsLight();
				SetStyleLight();
			}
			else
			{
				ImGui::StyleColorsDark();
				SetStyleDark();
			}
		}

		ImGui::End();
	}

	void Editor::RenderViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

		ImGui::PushFont(GetFont(ImGuiFonts::BoldItalic));
		ImGui::Begin("Viewport");
		ImGui::PopFont();

		m_ViewportSize = *((glm::vec2*)&ImGui::GetContentRegionAvail()); // same memory layout
		m_ViewportPosition = ImGui::GetCursorScreenPos();

		auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos(); // Title bar
		m_ViewportBound[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBound[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		if (m_RuntimeRenderer->GetRuntimeRendererSpec().ApiSpec.MainFrameBufferSpec.Target == PresentTarget::CustomViewport)
		{
			if (m_MainViewportTexID != VK_NULL_HANDLE)
			{
				ImGui_ImplVulkan_RemoveTexture(m_MainViewportTexID);
				m_MainViewportTexID = VK_NULL_HANDLE;
			}

			VulkanImage* image = (VulkanImage*)m_RuntimeRenderer->GetFinalImage(m_Swapchain->GetImageIndex());
			m_MainViewportTexID = ImGui_ImplVulkan_AddTexture(image->TextureSampler, image->ColorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			ImGui::Image(m_MainViewportTexID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, { 0, 1 }, { 1, 0 }, ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Editor::BeginFrame()
	{
		CORE_PROFILER_FUNC();

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Editor::EndFrame()
	{
		CORE_PROFILER_FUNC();

		// Rendering
		ImGui::Render();

		VkClearValue clearValues[1];
		clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };

		uint32_t width = m_Swapchain->GetSwapChainExtent().width;
		uint32_t height = m_Swapchain->GetSwapChainExtent().height;

		uint32_t commandBufferIndex = m_Swapchain->GetCurrentFrame();

		VkCommandBuffer mainCommandBuffer = VulkanContext::GetVulkanDevice()->GetCurrentCommandBuffer();

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_ImGuiRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = s_ImGuiFramebuffer->GetFramebuffer(m_Swapchain->GetImageIndex());

		vkCmdBeginRenderPass(mainCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = m_ImGuiRenderPass;
		inheritanceInfo.framebuffer = s_ImGuiFramebuffer->GetFramebuffer(m_Swapchain->GetImageIndex());

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

		VK_CHECK_RESULT(vkBeginCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex], &cmdBufInfo));

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &scissor);

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_ImGuiCommandBuffers[commandBufferIndex]);

		VK_CHECK_RESULT(vkEndCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex]));

		std::array<VkCommandBuffer, 1> commandBuffers;
		commandBuffers[0] = (s_ImGuiCommandBuffers[commandBufferIndex]);

		vkCmdExecuteCommands(mainCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());

		vkCmdEndRenderPass(mainCommandBuffer);

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	std::pair<int, int> Editor::GetMousePosRelativeViewport(const ImVec2& viewportPos)
	{
		auto [mx, my] = ImGui::GetMousePos();
		glm::vec2 viewportSize = m_ViewportBound[1] - m_ViewportBound[0];

		int resultX = (int)(mx - viewportPos.x);
		int resultY = (int)(viewportSize.y - (my - m_ViewportBound[0].y));

		return { resultX ,resultY };
	}

	static bool PointInsideAABB(const glm::vec3& point, const AABB& aabb)
	{
		//return point.x >= aabb.Min.x && point.x <= aabb.Max.x &&
		//	point.y >= aabb.Min.y && point.y <= aabb.Max.y &&
		//	point.z >= aabb.Min.z && point.z <= aabb.Max.z;

		//vectorized comparison
		glm::bvec3 inside = glm::greaterThanEqual(point, aabb.Min) && glm::lessThanEqual(point, aabb.Max);

		// Check if all components are inside the AABB
		return inside.x && inside.y && inside.z;
	}

	void Editor::OnMousePressed(const MouseButtonPressedEvent& mouseEvent)
	{
		CORE_PROFILER_FUNC();

		auto [mx, my] = GetMousePosRelativeViewport(m_ViewportPosition);

		if (mx >= 0 && my >= 0 && mx < (int)m_ViewportSize.x && my < (int)m_ViewportSize.y)
		{
			uint32_t entityID = m_RuntimeRenderer->GetRenderAPI().As<VulkanAPI>()->GetMainFrameBuffer()->ReadPixelsFromImage(mx, my);

			if (entityID > 0)
			{
				Entity selectedEntity;
				selectedEntity.SetEntityId((EntityID)(entityID - 1));
				selectedEntity.SetWorld(m_CurrentWorld.StdSharedPtr.get());

				m_HierarchyPanel.SetSelectedEntity(selectedEntity);
			}
			else
			{
				m_HierarchyPanel.SetSelectedEntity({});
			}
		}
	}

	void Editor::SetupInput(EventHandler& eventHandler)
	{
		m_CurrentWorld->SetupInput(eventHandler);

		eventHandler.BindAction(EventType::MouseButtonPressed, this, &Editor::OnMousePressed);
	}

}

