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

	template <typename Callable>
	class DrawCommand {
	private:
		Callable m_Callable;

	public:
		DrawCommand(Callable callable) 
			: m_Callable(std::move(callable))
		{}

		void operator()()
		{
			m_Callable();
		}
	};


	class VulkanAPI : public IRenderAPI
	{
	public:
		virtual void Init(const RenderApiSpec& spec) override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void BeginMainPass() override;
		virtual void EndMainPass() override;

		virtual void DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection) override;
		
		virtual void WaitForIdle() override;
		virtual void ResizeFrameBuffer(bool resize = false, uint32_t width = 0, uint32_t height = 0) override;

		virtual void SetVSync(bool vsync) override;
		virtual void* GetFinalImage(uint32_t index = 0) override;

		inline static constexpr int GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
		inline const SharedPtr<RenderPass>& GetRenderPass() { return m_MainRenderPass; }
		inline const SharedPtr<PipeLine> GetGraphicsPipline() { return m_Pipline; }
	private:
		RenderApiSpec m_ApiSpec;

		SharedPtr<VulkanDevice> m_Device;
		SharedPtr<RenderPass> m_MainRenderPass;
		SharedPtr<VulkanFramebuffer> m_MainFrameBuffer;
		SharedPtr<PipeLine> m_Pipline;

		static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
		static bool s_ResizeFrameBuffer;
		static bool s_RecreateSwapChainRequested;

		friend class VulkanSwapChain;
	};
}
