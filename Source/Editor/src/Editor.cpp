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

namespace PAL
{
	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;
	static VkDescriptorPool s_DescriptorPool = VK_NULL_HANDLE;
	static VkSampler s_TextureSampler;
	static std::vector<VkDescriptorSet> s_Dset;

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
		window.Vsync = true;
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
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// Create Descriptor Pool
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

		ImGui_ImplVulkan_Init(&init_info, swapchain->GetRenderPass());

		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();
		s_ImGuiCommandBuffers.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			s_ImGuiCommandBuffers[i] = VulkanContext::GetVulkanDevice()->GetMainCommandBuffer()->CreateSecondaryCommandBuffer();
		}

		//VkSamplerCreateInfo samplerInfo{};
		//samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//samplerInfo.magFilter = VK_FILTER_LINEAR;
		//samplerInfo.minFilter = VK_FILTER_LINEAR;
		//samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//samplerInfo.anisotropyEnable = VK_FALSE;
		//samplerInfo.maxAnisotropy = 1.0f;
		//samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//samplerInfo.unnormalizedCoordinates = VK_FALSE;
		//samplerInfo.compareEnable = VK_FALSE;
		//samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		//samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		//samplerInfo.mipLodBias = 0.0f;
		//samplerInfo.minLod = 0.0f;
		//samplerInfo.maxLod = 0.0f;
		//VK_CHECK_RESULT(vkCreateSampler(VulkanContext::GetVulkanDevice()->GetVkDevice(), &samplerInfo, nullptr, &s_TextureSampler));

		//s_Dset.resize(swapchain->GetSwapChainImageViews().size());
		//for (uint32_t i = 0; i < swapchain->GetSwapChainImageViews().size(); i++)
		//	s_Dset[i] = ImGui_ImplVulkan_AddTexture(s_TextureSampler, swapchain->GetSwapChainImageViews().at(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
	}

	void Editor::OnRenderImGui()
	{
		BeginFrame();
		{
			static bool show_demo_window = true;
			ImGui::ShowDemoWindow(&show_demo_window);

			ImGui::Begin("Viewport");

			auto& swapChain = VulkanContext::GetSwapChain();

			//ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			//ImGui::Image(s_Dset[VulkanAPI::GetCurrentFrame()], ImVec2{ viewportPanelSize.x, viewportPanelSize.y });

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

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapChain->GetSwapChainExtent().width;
		uint32_t height = swapChain->GetSwapChainExtent().height;

		uint32_t commandBufferIndex = VulkanAPI::GetCurrentFrame();

		VkCommandBuffer mainCommandBuffer = VulkanContext::GetVulkanDevice()->GetCurrentCommandBuffer();

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2; // Color + depth
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = swapChain->GetSwapChainFramebuffers().at(swapChain->GetImageIndex());

		vkCmdBeginRenderPass(mainCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = swapChain->GetRenderPass();
		inheritanceInfo.framebuffer = swapChain->GetSwapChainFramebuffers().at(swapChain->GetImageIndex());

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

