#pragma once
#include "VulkanAPI.h"

namespace PAL
{
	class VulkanDevice;
	class VulkanFramebuffer;

	class RenderPass
	{
	public:
		RenderPass(const SharedPtr<VulkanDevice>& vulkanDevice, bool isSwapchainTarget);

		void Shutdown();

		void Begin(VkCommandBuffer currentCommandBuffer, uint32_t imageIndex);
		void End(VkCommandBuffer currentCommandBuffer);

		inline VkRenderPass GetVkRenderPass() { return m_RenderPass; }
		inline const VkRenderPass GetVkRenderPass() const { return m_RenderPass; }

		inline void SetFrameBuffer(const SharedPtr<VulkanFramebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		inline bool IsSwapchainTarget() const { return m_IsSwapchainTarget; }
	private:
		VkRenderPass m_RenderPass;

		SharedPtr<VulkanFramebuffer> m_Framebuffer;

		bool m_IsSwapchainTarget;
	};

}