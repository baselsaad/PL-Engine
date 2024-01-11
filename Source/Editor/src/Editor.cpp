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
		window.Width = 800.0f;
		window.Height = 600.0f;
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

		// Upload Font
		{
			//VkCommandBuffer command_buffer = beginSingleTimeCommands();
			//ImGui_ImplVulkan_CreateFontsTexture();
			//endSingleTimeCommands(command_buffer);
		}

		CreateRenderPass();

		FramebufferSpecification imguiSpec = {};
		imguiSpec.BufferCount = swapchain->GetSwapChainImages().size();
		imguiSpec.ColorFormat = swapchain->GetSwapChainImageFormat();
		imguiSpec.DepthFormat = VK_FORMAT_UNDEFINED;
		imguiSpec.Width = 800;
		imguiSpec.Height = 600;
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

	VkDescriptorSet s_TextureID; 
	void Editor::OnRenderImGui(VulkanImage* image)
	{
		BeginFrame();
		{
			//static bool show_demo_window = true;
			//ImGui::ShowDemoWindow(&show_demo_window);

			ImGui::Begin("Viewport");

			m_ViewportSize = ImGui::GetContentRegionAvail();
			auto& sceneFramebuffer = Engine::Get()->GetRenderer()->GetRenderAPI().As<VulkanAPI>()->GetSceneFrameBuffer();
			auto& framebufferSpec = sceneFramebuffer->GetSpecification();

			if (!framebufferSpec.IsSwapchainTarget)
			{
				if (image->ImageLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				{
					VulkanContext::GetSwapChain()->TransitionImageLayout(image->ColorImage, image->ImageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					image->ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

				if (s_TextureID != VK_NULL_HANDLE)
				{
					ImGui_ImplVulkan_RemoveTexture(s_TextureID);
					s_TextureID = VK_NULL_HANDLE;
				}

				s_TextureID = ImGui_ImplVulkan_AddTexture(image->TextureSampler, image->ColorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				ImGui::Image(s_TextureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y });
			}

			ImGui::End();
		}
		EndFrame();
	}

	void Editor::BeginFrame()
	{
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Editor::EndFrame()
	{
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

