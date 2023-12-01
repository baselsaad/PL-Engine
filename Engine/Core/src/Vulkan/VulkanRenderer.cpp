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

		//const std::vector<Vertex> vertices =
		//{
		//	// First Quad
		//	{ {-0.5f, -0.5f,0.0f}, {1.0f, 0.0f, 0.0f} },
		//	{ {0.5f, -0.5f ,0.0f},  {0.0f, 1.0f, 0.0f} },
		//	{ {0.5f, 0.5f  ,0.0f},   {0.0f, 0.0f, 1.0f} },
		//	{ {-0.5f, 0.5f ,0.0f},  {1.0f, 1.0f, 1.0f} },

		//	// Second Quad (example: shifted to the right)
		//	{ {0.5f, -0.5f, 0.0f},  {0.0f, 1.0f, 1.0f} },
		//	{ {1.5f, -0.5f, 0.0f},  {1.0f, 0.0f, 1.0f} },
		//	{ {1.5f,  0.5f, 0.0f},   {1.0f, 1.0f, 0.0f} },
		//	{ {0.5f,  0.5f, 0.0f},   {0.5f, 0.5f, 0.5f} }
		//};

		//const std::vector<uint16_t> indices =
		//{
		//	// First Quad
		//	0, 1, 2,
		//	2, 3, 0,

		//	// Second Quad
		//	4, 5, 6,
		//	6, 7, 4
		//};
		//m_IndexBuffer = MakeShared<VulkanIndexBuffer>(m_CommandBuffer, (void*)indices.data(), indices.size());
		//m_VertexBuffer = MakeShared<VulkanVertexBuffer>(m_CommandBuffer, (void*)vertices.data(), vertices.size() * sizeof(Vertex));

		m_IndexBuffer = MakeShared<VulkanIndexBuffer>(m_CommandBuffer);
		m_VertexBuffer = MakeShared<VulkanVertexBuffer>(m_CommandBuffer);

		CreateSyncObjects();
	}

	void VulkanAPI::InitRenderApiContext()
	{
		VulkanContext::Init();
		VulkanContext::CreateVulkanSwapChain();
	}

	void VulkanAPI::Shutdown()
	{
		//Delete Semaphores
		for (size_t i = 0; i < VulkanAPI::GetMaxFramesInFlight(); i++)
		{
			vkDestroySemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_RenderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_ImageAvailableSemaphore[i], nullptr);
			vkDestroyFence(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_InFlightFence[i], nullptr);
		}

		m_CommandBuffer->Shutdown();
		m_Pipline->Shutdown();
		m_RenderPass->Shutdown();

		m_IndexBuffer->DestroyBuffer();
		m_VertexBuffer->DestroyBuffer();

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

	void VulkanAPI::DrawTriangle()
	{
		DrawFrame();
	}

	void VulkanAPI::DrawFrame()
	{
		const SharedPtr<VulkanSwapChain>& swapchain = VulkanContext::GetSwapChain();

		vkWaitForFences(VulkanContext::GetVulkanDevice()->GetVkDevice(), 1, &m_InFlightFence[s_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(VulkanContext::GetVulkanDevice()->GetVkDevice(), swapchain->GetVkSwapChain(), UINT64_MAX, m_ImageAvailableSemaphore[s_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			swapchain->RecreateSwapChain(m_RenderPass);
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			ASSERT(false, "failed to acquire swap chain image!");
		}

		// Only reset the fence if we are submitting work
		vkResetFences(VulkanContext::GetVulkanDevice()->GetVkDevice(), 1, &m_InFlightFence[s_CurrentFrame]);

		const auto& commandBuffers = m_CommandBuffer->GetCommandBuffers();

		vkResetCommandBuffer(commandBuffers[s_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

		m_CommandBuffer->SubmitCommandBuffer(commandBuffers[s_CurrentFrame], imageIndex, m_RenderPass, swapchain, m_Pipline,
			m_IndexBuffer, m_VertexBuffer, m_IndexBuffer->GetCount());// Actual Drawing and binding Commands

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[s_CurrentFrame];

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore[s_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore[s_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(VulkanContext::GetVulkanDevice()->GetVkGraphicsQueue(), 1, &submitInfo, m_InFlightFence[s_CurrentFrame])); // execution of the recorded commands

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapchain->GetVkSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(VulkanContext::GetVulkanDevice()->GetVkPresentQueue(), &presentInfo);

		const auto& window = Engine::Get()->GetWindow();

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || s_ResizeFrameBuffer)
		{
			s_ResizeFrameBuffer = false; // reset
			swapchain->RecreateSwapChain(m_RenderPass);
		}
		else if (result != VK_SUCCESS)
		{
			ASSERT(false, "failed to present swap chain image!");
		}

		s_CurrentFrame = (s_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanAPI::CreateSyncObjects()
	{
		m_ImageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Each Frame has it's own Semaphores and Fences
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(VulkanContext::GetVulkanDevice()->GetVkDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]));
			VK_CHECK_RESULT(vkCreateFence(VulkanContext::GetVulkanDevice()->GetVkDevice(), &fenceInfo, nullptr, &m_InFlightFence[i]));
		}
	}
}