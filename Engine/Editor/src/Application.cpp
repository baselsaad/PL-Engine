#include "pch.h"
#include "Shader.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include "Vulkan/VulkanContext.h"
#include "Event/Event.h"
#include "Event/PlayerInput.h"
#include "Core/App.h"
#include "Vulkan/SwapChain.h"

#include "Vulkan/VulkanDevice.h"
#include "Core/Window.h"

namespace PL_Engine
{
	inline static SharedPtr<VulkanSwapChain> s_SwapChain;
	inline static SharedPtr<VulkanDevice> s_VulkanDevice;
	inline static SharedPtr<VulkanPhysicalDevice> s_VulkanPhysicalDevice;

	inline static GLFWwindow* s_Window;

	// Pipeline 
	static VkRenderPass s_RenderPass;
	static VkPipelineLayout s_PipelineLayout;
	static VkPipeline s_GraphicsPipeline;

	// Command Buffer
	static VkCommandPool s_CommandPool;
	static std::vector<VkCommandBuffer> s_CommandBuffers;

	// semaphores
	static std::vector<VkSemaphore> s_ImageAvailableSemaphore;
	static std::vector<VkSemaphore> s_RenderFinishedSemaphore;
	static std::vector<VkFence> s_InFlightFence;

	// Frames
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	static uint32_t s_CurrentFrame = 0;

	static VkShaderModule CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(s_VulkanDevice->GetVkDevice(), &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	static void CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = s_SwapChain->GetSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		VK_CHECK_RESULT(vkCreateRenderPass(s_VulkanDevice->GetVkDevice(), &renderPassInfo, nullptr, &s_RenderPass));
	}

	static void CreateGraphicsPipeline()
	{
		auto vertShaderCode = Shader::ReadFile("res/shaders/vert.spv");
		auto fragShaderCode = Shader::ReadFile("res/shaders/frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		//Shader stage creation
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		/*
		*	VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
		*	VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
		*	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
		*	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
		*	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
		*/
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(vkCreatePipelineLayout(s_VulkanDevice->GetVkDevice(), &pipelineLayoutInfo, nullptr, &s_PipelineLayout));

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = s_PipelineLayout;
		pipelineInfo.renderPass = s_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(s_VulkanDevice->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &s_GraphicsPipeline));

		vkDestroyShaderModule(s_VulkanDevice->GetVkDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(s_VulkanDevice->GetVkDevice(), vertShaderModule, nullptr);
	}

	static void RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(s_Window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(s_Window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(s_VulkanDevice->GetVkDevice());// wait for resources

		s_SwapChain->CleanupSwapChain();

		s_SwapChain->Create();
		s_SwapChain->CreateImageViews();
		s_SwapChain->CreateFramebuffers(s_RenderPass);
	}

	static void CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = s_VulkanPhysicalDevice->FindQueueFamilies();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		VK_CHECK_RESULT(vkCreateCommandPool(s_VulkanDevice->GetVkDevice(), &poolInfo, nullptr, &s_CommandPool));
	}

	static void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = s_RenderPass;
		renderPassInfo.framebuffer = s_SwapChain->GetSwapChainFramebuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = s_SwapChain->GetSwapChainExtent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_GraphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(s_SwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(s_SwapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = s_SwapChain->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

	static void CreateCommandBuffer()
	{
		// each frame has it's own command buffer
		s_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = s_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)s_CommandBuffers.size();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(s_VulkanDevice->GetVkDevice(), &allocInfo, s_CommandBuffers.data()));
	}

	static void DrawFrame()
	{
		vkWaitForFences(s_VulkanDevice->GetVkDevice(), 1, &s_InFlightFence[s_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(s_VulkanDevice->GetVkDevice(), s_SwapChain->GetVkSwapChain(), UINT64_MAX, s_ImageAvailableSemaphore[s_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			ASSERT(false, "failed to acquire swap chain image!");
		}

		// Only reset the fence if we are submitting work
		vkResetFences(s_VulkanDevice->GetVkDevice(), 1, &s_InFlightFence[s_CurrentFrame]);

		vkResetCommandBuffer(s_CommandBuffers[s_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		RecordCommandBuffer(s_CommandBuffers[s_CurrentFrame], imageIndex);// Actual Drawing and binding Commands

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &s_CommandBuffers[s_CurrentFrame];

		VkSemaphore waitSemaphores[] = { s_ImageAvailableSemaphore[s_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkSemaphore signalSemaphores[] = { s_RenderFinishedSemaphore[s_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(s_VulkanDevice->GetVkGraphicsQueue(), 1, &submitInfo, s_InFlightFence[s_CurrentFrame]));

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { s_SwapChain->GetVkSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(s_VulkanDevice->GetVkPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || App::s_FramebufferResized)
		{
			App::s_FramebufferResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			ASSERT(false, "failed to present swap chain image!");
		}

		s_CurrentFrame = (s_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	static void CreateSyncObjects()
	{
		s_ImageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		s_RenderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		s_InFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Each Frame has it's own Semaphores and Fences
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(s_VulkanDevice->GetVkDevice(), &semaphoreInfo, nullptr, &s_ImageAvailableSemaphore[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(s_VulkanDevice->GetVkDevice(), &semaphoreInfo, nullptr, &s_RenderFinishedSemaphore[i]));
			VK_CHECK_RESULT(vkCreateFence(s_VulkanDevice->GetVkDevice(), &fenceInfo, nullptr, &s_InFlightFence[i]));
		}
	}

	static VkInstance InitVulkan()
	{
		// Create Instance 
		s_Window = App::Get()->GetWindow()->GetWindowHandle();

		VulkanContext::Init();
		VulkanContext::CreateSurface();
		VkInstance instance = PL_Engine::VulkanContext::GetVulkanInstance();

		s_VulkanPhysicalDevice = MakeShared<VulkanPhysicalDevice>();
		s_VulkanDevice = MakeShared<VulkanDevice>(s_VulkanPhysicalDevice);

		s_SwapChain = MakeShared<VulkanSwapChain>(s_VulkanDevice);
		s_SwapChain->Create();
		s_SwapChain->CreateImageViews();


		CreateRenderPass();
		CreateGraphicsPipeline();
		s_SwapChain->CreateFramebuffers(s_RenderPass);

		CreateCommandPool();
		CreateCommandBuffer();
		CreateSyncObjects();

		return instance;
	}

	static void Cleanup(VkInstance instance)
	{
		//Vulkan
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(s_VulkanDevice->GetVkDevice(), s_RenderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(s_VulkanDevice->GetVkDevice(), s_ImageAvailableSemaphore[i], nullptr);
			vkDestroyFence(s_VulkanDevice->GetVkDevice(), s_InFlightFence[i], nullptr);
		}

		vkDestroyCommandPool(s_VulkanDevice->GetVkDevice(), s_CommandPool, nullptr);

		vkDestroyPipeline(s_VulkanDevice->GetVkDevice(), s_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(s_VulkanDevice->GetVkDevice(), s_PipelineLayout, nullptr);
		vkDestroyRenderPass(s_VulkanDevice->GetVkDevice(), s_RenderPass, nullptr);

		s_SwapChain->CleanupSwapChain();

		vkDestroyDevice(s_VulkanDevice->GetVkDevice(), nullptr);
		vkDestroySurfaceKHR(instance, VulkanContext::GetSurface(), nullptr);
	}

	static void MainLoop()
	{
		SharedPtr<App> app = MakeShared<App>();
		VkInstance instance = InitVulkan();

		while (!glfwWindowShouldClose(s_Window))
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(s_VulkanDevice->GetVkDevice());
		Cleanup(instance);
	}

}

int main()
{
	Debug::Init("Engine-Editor");
	PL_Engine::MainLoop();
}