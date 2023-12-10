#pragma once
#include "VulkanAPI.h"

namespace PAL
{
	class PipeLine;
	class VulkanSwapChain;
	class RenderPass;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;


	class CommandBuffer
	{
	public:
		CommandBuffer();
		void Shutdown();

		void CreateCommandPool();
		void CreateCommandBuffer();

		inline const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_CommandBuffers; }
		inline const VkCommandPool GetCommandPool() const { return m_CommandPool; }

	private:
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};


}