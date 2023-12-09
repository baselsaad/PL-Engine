#pragma once
#include "VulkanAPI.h"
#include "Renderer/RenderAPI.h"


namespace PL_Engine
{
	class CommandBuffer;
	class PipeLine;
	class RenderPass;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class VulkanAPI : public IRenderAPI
	{
	public:
		void Init();
		virtual void InitRenderApiContext() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection) override;
		
		virtual void WaitForIdle() override;
		virtual void OnResizeWindow(bool resize = false, int width = 0, int height = 0) override;

		virtual void SubmitCommand(const std::function<void()>& drawCommand);

		inline static constexpr int GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
		inline static uint32_t GetCurrentFrame() { return s_CurrentFrame; }
		inline const SharedPtr<RenderPass>& GetRenderPass() { return m_RenderPass; }
		inline const SharedPtr<PipeLine> GetGraphicsPipline() { return m_Pipline; }
		inline const SharedPtr<CommandBuffer>& GetCommandBuffer() { return m_CommandBuffer; }

	private:
		void ExcuteDrawCommands();

	private:
		SharedPtr<RenderPass> m_RenderPass;
		SharedPtr<PipeLine> m_Pipline;
		SharedPtr<CommandBuffer> m_CommandBuffer;

		std::vector <std::function<void()>> m_Commands;

		static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
		static uint32_t s_CurrentFrame;
		static bool s_ResizeFrameBuffer;

		friend class VulkanSwapChain;
	};
}
