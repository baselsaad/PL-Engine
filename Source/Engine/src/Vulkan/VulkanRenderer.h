#pragma once
#include "VulkanAPI.h"
#include "Renderer/RenderAPI.h"


namespace PAL
{
	class CommandBuffer;
	class PipeLine;
	class RenderPass;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class VulkanFramebuffer;

	class VulkanAPI : public IRenderAPI
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void FlushDrawCommands() override;

		virtual void DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection) override;
		
		virtual void WaitForIdle() override;
		virtual void ResizeFrameBuffer(bool resize = false, uint32_t width = 0, uint32_t height = 0) override;

		virtual void RecordDrawCommand(const std::function<void()>& drawCommand);
		virtual void PresentFrame() override;
		virtual void SetVSync(bool vsync) override;

		inline static constexpr int GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
		inline static uint32_t GetCurrentFrame() { return s_CurrentFrame; }
		inline const SharedPtr<RenderPass>& GetRenderPass() { return m_RenderPass; }
		inline const SharedPtr<PipeLine> GetGraphicsPipline() { return m_Pipline; }

		// Remove Later
		inline const SharedPtr<VulkanFramebuffer>& GetSceneFrameBuffer() { return m_SceneFrameBuffer; }

	private:
		SharedPtr<RenderPass> m_RenderPass;
		SharedPtr<PipeLine> m_Pipline;
		SharedPtr<VulkanFramebuffer> m_SceneFrameBuffer;
		std::vector <std::function<void()>> m_DrawCommands;

		SharedPtr<VulkanDevice> m_Device;

		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
		static uint32_t s_CurrentFrame;
		static bool s_ResizeFrameBuffer;
		static bool s_RecreateSwapChainRequested;

		friend class VulkanSwapChain;
	};
}
