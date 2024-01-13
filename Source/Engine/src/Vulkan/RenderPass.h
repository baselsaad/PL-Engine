#pragma once
#include "VulkanAPI.h"
#include "Renderer/RenderAPI.h"

namespace PAL
{
	class VulkanDevice;
	class VulkanFramebuffer;

	class RenderPass
	{
	public:
		RenderPass(const SharedPtr<VulkanDevice>& vulkanDevice, const RenderpassSpecification& renderpassSpec);

		void Shutdown();

		void Begin(VkCommandBuffer currentCommandBuffer, uint32_t imageIndex);
		void End(VkCommandBuffer currentCommandBuffer);

		inline VkRenderPass GetVkRenderPass() { return m_RenderPass; }
		inline const VkRenderPass GetVkRenderPass() const { return m_RenderPass; }

		inline void SetFrameBuffer(const SharedPtr<VulkanFramebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		inline const  RenderpassSpecification& GetRenderpassSpec() const { return m_RenderpassSpec; }
	private:
		VkRenderPass m_RenderPass;

		SharedPtr<VulkanFramebuffer> m_Framebuffer;
		RenderpassSpecification m_RenderpassSpec;
	};

}