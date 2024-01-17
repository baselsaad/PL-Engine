#include "pch.h"

#include "CommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "GraphhicsPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"

namespace PAL
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

	VkCommandBuffer CommandBuffer::CreateSecondaryCommandBuffer() const
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = m_CommandPool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanContext::GetVulkanDevice()->GetVkDevice(), &cmdBufAllocateInfo, &cmdBuffer));
		return cmdBuffer;
	}
}