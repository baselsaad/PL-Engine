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
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/VulkanContext.h"


#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "Vulkan/VulkanRenderer.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderAPI.h"
#include "Vulkan/VulkanFramebuffer.h"

namespace PAL
{
	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
	static SharedPtr<VulkanFramebuffer> s_ImGuiFramebuffer;
	static VkDescriptorPool s_DescriptorPool = VK_NULL_HANDLE;

	int ApplicationMain(int argc, char** args)
	{
		EngineArgs engineArgs = {};
		engineArgs.ArgumentsCount = argc;
		engineArgs.RawArgumentStrings = args;
		engineArgs.AppName = "Editor";

		WindowData window;
		window.Width = 1600;
		window.Height = 900;
		window.Title = "PAL Editor";
		window.Vsync = false;
		window.Mode = WindowMode::Windowed;

		engineArgs.EngineWindowData = window;

		BEGIN_PROFILE_SESSION("Init Engine Session");
		PAL::Engine engine(engineArgs);
		END_PROFILE_SESSION();

		BEGIN_PROFILE_SESSION("EngineLoop Session");
		engine.Run();
		END_PROFILE_SESSION();

		BEGIN_PROFILE_SESSION("Engine Shutdown Session");
		engine.Stop();
		END_PROFILE_SESSION();

		return 0;
	}

	Editor::Editor()
	{
		auto swapchain = VulkanContext::GetSwapChain();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		SetDarkThemeColors();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

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

		CreateRenderPass();

		FramebufferSpecification imguiSpec = {};
		imguiSpec.BufferCount = swapchain->GetSwapChainImages().size();
		imguiSpec.ColorFormat = swapchain->GetSwapChainImageFormat();
		imguiSpec.DepthFormat = VK_FORMAT_UNDEFINED;
		imguiSpec.Width = swapchain->GetSwapChainExtent().width;
		imguiSpec.Height = swapchain->GetSwapChainExtent().height;
		imguiSpec.UseDepth = false;
		imguiSpec.IsSwapchainTarget = true;
		imguiSpec.DebugName = "imgui Framebuffer";

		s_ImGuiFramebuffer = NewShared<VulkanFramebuffer>(VulkanContext::GetVulkanDevice()->GetVkDevice(),
			VulkanContext::GetVulkanDevice()->GetPhysicalDevice()->GetVkPhysicalDevice()
			, m_ImGuiRenderPass, imguiSpec);

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
		init_info.ImageCount = swapchain->GetSwapChainImages().size();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;

		ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass);

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

			m_ImGuiFonts.emplace(ImGuiFonts::Regular, regularFont);
			m_ImGuiFonts.emplace(ImGuiFonts::Bold, boldFont);
			m_ImGuiFonts.emplace(ImGuiFonts::Light, lightFont);
			m_ImGuiFonts.emplace(ImGuiFonts::BoldItalic, boldItalicFont);
			m_ImGuiFonts.emplace(ImGuiFonts::RegularItalic, regularItalicFont);
			m_ImGuiFonts.emplace(ImGuiFonts::LightItalic, lightItalicFont);

			ImGui_ImplVulkan_CreateFontsTexture();
		}

		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();
		s_ImGuiCommandBuffers.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			s_ImGuiCommandBuffers[i] = VulkanContext::GetVulkanDevice()->GetMainCommandBuffer()->CreateSecondaryCommandBuffer();
		}
	}

	Editor::~Editor()
	{
	}

	Editor& Editor::GetInstance()
	{
		static Editor instance;

		return instance;
	}

	void Editor::Shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(VulkanContext::GetVulkanDevice()->GetVkDevice(), s_DescriptorPool, nullptr);
		vkDestroyRenderPass(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_ImGuiRenderPass, nullptr);
		s_ImGuiFramebuffer->Shutdown();
	}

	void Editor::CreateRenderPass()
	{
		VkAttachmentDescription attachments[1];
		attachments[0].format = VulkanContext::GetSwapChain()->GetSwapChainImageFormat();
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

		// if DONOT_CARE the ui will be rendered on top of the prev one (the prev one is responsible for clearing)
		// @TODO: Create a render pass just for clearing 

		if (Engine::Get()->GetRenderer()->GetRenderAPI().As<VulkanAPI>()->GetSceneFrameBuffer()->GetSpecification().IsSwapchainTarget)
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		else
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

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

	void Editor::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	VkDescriptorSet s_TextureID;
	void Editor::OnRenderImGui(VulkanImage* image)
	{
		CORE_PROFILER_FUNC();

		// Note: Switch this to true to enable dockspace
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
					Engine::Get()->Exit();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Stats View
		{
			ImGui::Begin("Stats");

			RenderStats& stats = Renderer::GetStats();
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.Quads);
			ImGui::Text("VertexBufferCount: %d", stats.VertexBufferCount);
			ImGui::Text("FrameTime : %.4fs", stats.FrameTime);
			ImGui::Text("FrameTime : %.4fms", stats.FrameTime_ms);
			ImGui::Text("FPS: %d", stats.FramesPerSecond);

			ImGui::End();
		}

		// Settings View
		{
			ImGui::Begin("Settings");
			auto& window = Engine::Get()->GetWindow();

			bool vsync = window->IsVsyncOn();
			ImGui::Checkbox("VSync", &vsync);
			if (vsync != window->IsVsyncOn())
				Engine::Get()->SetVSync(vsync);

			ImGui::End();
		}

		// World Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

			ImGui::PushFont(GetFont(ImGuiFonts::BoldItalic));
			ImGui::Begin("Viewport");
			ImGui::PopFont();

			auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto viewportOffset = ImGui::GetWindowPos();

			m_ViewportSize = ImGui::GetContentRegionAvail();

			auto& sceneFramebuffer = Engine::Get()->GetRenderer()->GetRenderAPI().As<VulkanAPI>()->GetSceneFrameBuffer();
			auto& framebufferSpec = sceneFramebuffer->GetSpecification();

			if (!framebufferSpec.IsSwapchainTarget)
			{
				if (s_TextureID != VK_NULL_HANDLE)
				{
					ImGui_ImplVulkan_RemoveTexture(s_TextureID);
					s_TextureID = VK_NULL_HANDLE;
				}

				s_TextureID = ImGui_ImplVulkan_AddTexture(image->TextureSampler, image->ColorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				ImGui::Image(s_TextureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y });
			}

			if (ImGui::BeginDragDropTarget())
			{
				//if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				//{
				//	const wchar_t* path = (const wchar_t*)payload->Data;
				//	OpenScene(std::filesystem::path(g_AssetPath) / path);
				//}
				ImGui::EndDragDropTarget();
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}

		ImGui::End();

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

		auto swapChain = VulkanContext::GetSwapChain();

		// Rendering
		ImGui::Render();

		VkClearValue clearValues[1];
		clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };

		uint32_t width = swapChain->GetSwapChainExtent().width;
		uint32_t height = swapChain->GetSwapChainExtent().height;

		uint32_t commandBufferIndex = VulkanAPI::GetCurrentFrame();

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
		renderPassBeginInfo.framebuffer = s_ImGuiFramebuffer->GetFramebuffer(swapChain->GetImageIndex());

		vkCmdBeginRenderPass(mainCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = m_ImGuiRenderPass;
		inheritanceInfo.framebuffer = s_ImGuiFramebuffer->GetFramebuffer(swapChain->GetImageIndex());

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

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(s_ImGuiCommandBuffers[commandBufferIndex]);

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

}

