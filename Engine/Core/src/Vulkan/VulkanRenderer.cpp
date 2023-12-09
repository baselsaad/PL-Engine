#include "pch.h"
#include "VulkanRenderer.h"

#include "GraphhicsPipeline.h"
#include "RenderPass.h"
#include "Shader.h"
#include "SwapChain.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "CommandBuffer.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Renderer/Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "glm/gtx/quaternion.hpp"
#include "Utilities/Timer.h"

namespace PL_Engine
{
	uint32_t VulkanAPI::s_CurrentFrame = 0;
	bool VulkanAPI::s_ResizeFrameBuffer = false;


	void VulkanAPI::Init()
	{
		m_RenderPass = MakeShared<RenderPass>(VulkanContext::GetVulkanDevice());
		m_Pipline = MakeShared<PipeLine>(m_RenderPass);
		VulkanContext::GetSwapChain()->CreateFramebuffers(m_RenderPass->GetVkRenderPass());
		m_CommandBuffer = MakeShared<CommandBuffer>();
	}

	void VulkanAPI::InitRenderApiContext()
	{
		VulkanContext::Init();
		VulkanContext::CreateVulkanSwapChain();
	}

	void VulkanAPI::Shutdown()
	{
		m_CommandBuffer->Shutdown();
		m_Pipline->Shutdown();
		m_RenderPass->Shutdown();

		VulkanMemoryAllocator::Shutdown();
		VulkanContext::Shutdown();
	}

	void VulkanAPI::WaitForIdle()
	{
		vkDeviceWaitIdle(VulkanContext::GetVulkanDevice()->GetVkDevice());
	}

	void VulkanAPI::OnResizeWindow(bool resize /*= false*/, int width /*= 0*/, int height /*= 0*/)
	{
		s_ResizeFrameBuffer = resize;
	}

	void VulkanAPI::SubmitCommand(const std::function<void()>& command)
	{
		m_Commands.push_back(command);
	}

	void VulkanAPI::BeginFrame()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();
		const auto& commandBuffers = m_CommandBuffer->GetCommandBuffers();

		// TODO: Move later
		{
			VulkanContext::GetSwapChain()->AcquireNextImage(m_RenderPass);
			vkResetCommandBuffer(commandBuffers[s_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers[s_CurrentFrame], &beginInfo));

		vkCmdBindPipeline(commandBuffers[s_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipline->GetGraphicsPipeline());
	}

	void VulkanAPI::EndFrame()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();
		const auto& commandBuffers = m_CommandBuffer->GetCommandBuffers();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(swapchain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[s_CurrentFrame], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffers[s_CurrentFrame], 0, 1, &scissor);

		// CommandBuffer
		{
			m_RenderPass->Begin(commandBuffers[s_CurrentFrame], swapchain->GetImageIndex());
			ExcuteDrawCommands();
			m_RenderPass->End(commandBuffers[s_CurrentFrame]);

			VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[s_CurrentFrame]));
		}

		VulkanContext::GetSwapChain()->PresentFrame(m_RenderPass, m_CommandBuffer); //Move later the main loop 
	}

	void VulkanAPI::DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection)
	{
		auto drawCommand = [this, vertexBuffer, indexBuffer, indexCount, projection]() -> void
		{
			const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();
			const auto& commandBuffers = m_CommandBuffer->GetCommandBuffers();

			// Bind VertexBuffer
			VkBuffer vertexBuffers[] = { vertexBuffer->GetVkVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[s_CurrentFrame], 0, 1, vertexBuffers, offsets);

			// Bind IndexBuffer
			vkCmdBindIndexBuffer(commandBuffers[s_CurrentFrame], indexBuffer->GetVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdPushConstants(commandBuffers[s_CurrentFrame], m_Pipline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &projection);
			vkCmdDrawIndexed(commandBuffers[s_CurrentFrame], indexCount, 1, 0, 0, 0);
		};

		SubmitCommand(drawCommand);
	}

	void VulkanAPI::ExcuteDrawCommands()
	{
		for (auto& func : m_Commands)
		{
			if (func)
				func();
		}

		m_Commands.clear();
		//m_Commands.resize(100);
	}

}