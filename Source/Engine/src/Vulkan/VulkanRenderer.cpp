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
#include "Renderer/RuntimeRenderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "glm/gtx/quaternion.hpp"
#include "Utilities/Timer.h"
#include "VulkanFramebuffer.h"

namespace PAL
{
	bool VulkanAPI::s_ResizeFrameBuffer = false;
	bool VulkanAPI::s_RecreateSwapChainRequested = false;

	void VulkanAPI::Init(const RenderApiSpec& spec)
	{
		m_ApiSpec = spec;

		m_Device = VulkanContext::GetVulkanDevice();
		VulkanMemoryAllocator::Init(VulkanContext::GetVulkanDevice());

		m_MainRenderPass = NewShared<RenderPass>(m_Device, spec.MainRenderpassSpec);
		m_MainFrameBuffer = NewShared<VulkanFramebuffer>(m_MainRenderPass->GetVkRenderPass(), spec.MainFrameBufferSpec);
		m_MainRenderPass->SetFrameBuffer(m_MainFrameBuffer);

		m_Pipline = NewShared<PipeLine>(m_MainRenderPass);
	}

	void VulkanAPI::Shutdown()
	{
		m_Pipline->Shutdown();
		m_MainRenderPass->Shutdown();
		m_MainFrameBuffer->Shutdown();

		VulkanMemoryAllocator::Shutdown();
		
	}

	void VulkanAPI::WaitForIdle()
	{
		vkDeviceWaitIdle(VulkanContext::GetVulkanDevice()->GetVkDevice());
	}

	void VulkanAPI::ResizeFrameBuffer(bool resize /*= false*/, uint32_t width /*= 0*/, uint32_t height /*= 0*/)
	{
		s_ResizeFrameBuffer = resize;

		if ((width > 0 && height > 0) &&
			(width != m_MainFrameBuffer->GetSpecification().Width || height != m_MainFrameBuffer->GetSpecification().Height))
		{
			m_MainFrameBuffer->Resize(width, height);
		}
	}

	void VulkanAPI::SetVSync(bool vsync)
	{
		s_RecreateSwapChainRequested = true;
	}

	void* VulkanAPI::GetFinalImage(uint32_t index)
	{
		return m_MainFrameBuffer->GetFrameBufferImage(index);
	}

	void VulkanAPI::BeginFrame()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = Engine::Get()->GetWindow()->GetSwapChain();

		// TODO: Move later
		{
			Engine::Get()->GetWindow()->GetSwapChain()->AcquireNextImage(m_MainRenderPass);
			vkResetCommandBuffer(m_Device->GetCurrentCommandBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK_RESULT(vkBeginCommandBuffer(m_Device->GetCurrentCommandBuffer(), &beginInfo));

		if (m_ApiSpec.MainFrameBufferSpec.Target == PresentTarget::CustomViewport
			&& m_MainFrameBuffer->GetFrameBufferImage()->ImageLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			VulkanUtilities::TransitionImageLayout(m_MainFrameBuffer->GetFrameBufferImage()->ColorImage, m_MainFrameBuffer->GetFrameBufferImage()->ImageLayout,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			m_MainFrameBuffer->GetFrameBufferImage()->ImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	void VulkanAPI::EndFrame()
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(m_Device->GetCurrentCommandBuffer()));
	}

	void VulkanAPI::BeginMainPass()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = Engine::Get()->GetWindow()->GetSwapChain();

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

		m_MainRenderPass->Begin(m_Device->GetCurrentCommandBuffer(), swapchain->GetImageIndex());
	}

	void VulkanAPI::EndMainPass()
	{
		m_MainRenderPass->End(m_Device->GetCurrentCommandBuffer());
	}

	void VulkanAPI::DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection)
	{
		const SharedPtr<VulkanSwapChain>& swapchain = Engine::Get()->GetWindow()->GetSwapChain();

		// Bind VertexBuffer
		VkBuffer vertexBuffers[] = { vertexBuffer->GetVkVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_Device->GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);

		// Bind IndexBuffer
		vkCmdBindIndexBuffer(m_Device->GetCurrentCommandBuffer(), indexBuffer->GetVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(m_Device->GetCurrentCommandBuffer(), m_Pipline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &projection);
		vkCmdDrawIndexed(m_Device->GetCurrentCommandBuffer(), indexCount, 1, 0, 0, 0);
	}
}