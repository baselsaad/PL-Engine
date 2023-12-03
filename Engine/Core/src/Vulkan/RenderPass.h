#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{
	class VulkanDevice;

	class RenderPass
	{
	public:
		RenderPass(const SharedPtr<VulkanDevice>& vulkanDevice);

		void Shutdown();

		void Begin(VkCommandBuffer currentCommandBuffer, uint32_t imageIndex);
		void End(VkCommandBuffer currentCommandBuffer);

		inline VkRenderPass GetVkRenderPass() { return m_RenderPass; }
		inline const VkRenderPass GetVkRenderPass() const { return m_RenderPass; }

	private:
		VkRenderPass m_RenderPass;
	};

}