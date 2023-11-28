#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{
	class PipeLine;
	class VulkanSwapChain;
	class RenderPass;


	class CommandBuffer
	{
	public:
		CommandBuffer();
		void Shutdown();

		void CreateCommandPool();
		void CreateCommandBuffer();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const SharedPtr<RenderPass>& renderpass, const SharedPtr<VulkanSwapChain>& swapChain, const SharedPtr<PipeLine>& graphicsPipline);

		inline const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_CommandBuffers; }
		inline const VkCommandPool GetCommandPool() const { return m_CommandPool; }

	private:
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};


}