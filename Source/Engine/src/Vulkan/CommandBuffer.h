#pragma once
#include "VulkanAPI.h"
#include "VulkanRenderer.h"

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

		VkCommandBuffer CreateSecondaryCommandBuffer() const;
		inline const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return m_CommandBuffers; }
		inline VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[VulkanAPI::GetCurrentFrame()]; }
		inline  VkCommandPool GetCommandPool() { return m_CommandPool; }

	private:
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};


}