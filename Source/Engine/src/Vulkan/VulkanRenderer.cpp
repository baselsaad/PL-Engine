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
#include "../../Editor/src/Editor.h"

namespace PAL
{
	uint32_t VulkanAPI::s_CurrentFrame = 0;
	bool VulkanAPI::s_ResizeFrameBuffer = false;

	void VulkanAPI::Init()
	{
		VulkanContext::Init();
		VulkanContext::CreateVulkanSwapChain();
		m_Device = VulkanContext::GetVulkanDevice();

		m_RenderPass = NewShared<RenderPass>(VulkanContext::GetVulkanDevice());
		VulkanContext::GetSwapChain()->CreateFramebuffers(m_RenderPass->GetVkRenderPass());

		m_Pipline = NewShared<PipeLine>(m_RenderPass);
	}

	void VulkanAPI::Shutdown()
	{
		m_Pipline->Shutdown();
		m_RenderPass->Shutdown();

		VulkanMemoryAllocator::Shutdown();
		VulkanContext::Shutdown();
	}

	void VulkanAPI::WaitForIdle()
	{
		vkDeviceWaitIdle(VulkanContext::GetVulkanDevice()->GetVkDevice());
	}

	void VulkanAPI::ResizeFrameBuffer(bool resize /*= false*/, int width /*= 0*/, int height /*= 0*/)
	{
		s_ResizeFrameBuffer = resize;
	}

	void VulkanAPI::RecordCommand(const std::function<void()>& command)
	{
		m_Commands.push_back(command);
	}

	void VulkanAPI::PresentFrame()
	{
		//VulkanContext::GetSwapChain()->TransitionImageLayout(m_CommandBuffer->GetCommandPool(), VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		VulkanContext::GetSwapChain()->PresentFrame(m_RenderPass, m_Device->GetMainCommandBuffer());
	}

	void VulkanAPI::BeginFrame()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();

		// TODO: Move later
		{
			VulkanContext::GetSwapChain()->AcquireNextImage(m_RenderPass);
			//VulkanContext::GetSwapChain()->TransitionImageLayout(m_CommandBuffer->GetCommandPool(), VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			vkResetCommandBuffer(m_Device->GetCurrentCommandBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK_RESULT(vkBeginCommandBuffer(m_Device->GetCurrentCommandBuffer(), &beginInfo));
	}

	void VulkanAPI::EndFrame()
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(m_Device->GetCurrentCommandBuffer()));
	}

	void VulkanAPI::FlushDrawCommands()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();

		vkCmdBindPipeline(m_Device->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipline->GetGraphicsPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(swapchain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_Device->GetCurrentCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain->GetSwapChainExtent();
		vkCmdSetScissor(m_Device->GetCurrentCommandBuffer(), 0, 1, &scissor);

		m_RenderPass->Begin(m_Device->GetCurrentCommandBuffer(), swapchain->GetImageIndex());
		{
			// Execute Draw Commands
			for (auto& func : m_Commands)
			{
				if (func)
					func();
			}

			m_Commands.clear();
		}
		m_RenderPass->End(m_Device->GetCurrentCommandBuffer());
	}

	void VulkanAPI::DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection)
	{
		auto drawCommand = [this, vertexBuffer, indexBuffer, indexCount, projection]() -> void
		{
			const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();

			// Bind VertexBuffer
			VkBuffer vertexBuffers[] = { vertexBuffer->GetVkVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_Device->GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);

			// Bind IndexBuffer
			vkCmdBindIndexBuffer(m_Device->GetCurrentCommandBuffer(), indexBuffer->GetVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdPushConstants(m_Device->GetCurrentCommandBuffer(), m_Pipline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &projection);
			vkCmdDrawIndexed(m_Device->GetCurrentCommandBuffer(), indexCount, 1, 0, 0, 0);
		};

		RecordCommand(drawCommand);
	}

}