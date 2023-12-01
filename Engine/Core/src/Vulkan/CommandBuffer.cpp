#include "pch.h"
#include "CommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "GraphhicsPipeline.h"
#include "VulkanRenderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace PL_Engine
{
	CommandBuffer::CommandBuffer()
	{
		CreateCommandPool();
		CreateCommandBuffer();
	}

	void CommandBuffer::Shutdown()
	{
		vkDestroyCommandPool(VulkanContext::GetVulkanDevice()->GetVkDevice(), m_CommandPool, nullptr);
	}

	void CommandBuffer::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = VulkanContext::GetVulkanDevice()->GetPhysicalDevice()->FindQueueFamilies();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		VK_CHECK_RESULT(vkCreateCommandPool(VulkanContext::GetVulkanDevice()->GetVkDevice(), &poolInfo, nullptr, &m_CommandPool));
	}

	void CommandBuffer::CreateCommandBuffer()
	{
		// each frame has it's own command buffer
		m_CommandBuffers.resize(VulkanAPI::GetMaxFramesInFlight());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::GetVulkanDevice()->GetVkDevice(), &allocInfo, m_CommandBuffers.data()));
	}

	void CommandBuffer::SubmitCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const SharedPtr<RenderPass>& renderpass, 
		const SharedPtr<VulkanSwapChain>& swapChain, const SharedPtr<PipeLine>& graphicsPipline, 
		const SharedPtr<VulkanIndexBuffer>& indexBuffer, const SharedPtr<VulkanVertexBuffer>& vertexBuffer, uint32_t indexBufferCount)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass->GetVkRenderPass();
		renderPassInfo.framebuffer = swapChain->GetSwapChainFramebuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetSwapChainExtent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipline->GetGraphicsPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChain->GetSwapChainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Bind VertexBuffer
		VkBuffer vertexBuffers[] = { vertexBuffer->GetVkVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// Bind IndexBuffer
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

		//vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertexBuffer->GetVertcies().size()), 1, 0, 0);
		vkCmdDrawIndexed(commandBuffer, indexBufferCount, 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

}