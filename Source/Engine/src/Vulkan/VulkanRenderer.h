#pragma once
#include "Renderer/RenderAPI.h"
#include "VulkanAPI.h"


namespace PAL
{
	class CommandBuffer;
	class PipeLine;
	class RenderPass;
	class VertexBuffer;
	class IndexBuffer;
	class VulkanFramebuffer;
	class VulkanDevice;
	class VulkanSwapChain;


	class VulkanAPI : public RenderAPI
	{
	public:
		virtual void Init(const RenderApiSpec& spec) override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void BeginMainPass() override;
		virtual void EndMainPass() override;

		virtual void PushConstant(void* data, int size, ShaderStage stage) override;

		virtual void DrawQuad(const SharedPtr<VertexBuffer>& vertexBuffer, const SharedPtr<IndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection) override;
		
		virtual void WaitForIdle() override;
		virtual void ResizeFrameBuffer(uint32_t width = 0, uint32_t height = 0, int frameIndex = -1) override;

		virtual void SetVSync(bool vsync) override;
		virtual void* GetFinalImage(uint32_t index = 0) override;

		inline static constexpr int GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
		inline const SharedPtr<RenderPass>& GetRenderPass() { return m_MainRenderPass; }
		inline const SharedPtr<VulkanFramebuffer>& GetMainFrameBuffer() { return m_MainFrameBuffer; }
		inline const SharedPtr<PipeLine> GetGraphicsPipline() { return m_Pipline; }


	private:
		//TODO: convert this to Macro as Config
		static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
		static bool s_RecreateSwapChainRequested;

		RenderApiSpec m_ApiSpec;

		SharedPtr<VulkanDevice> m_Device;
		SharedPtr<RenderPass> m_MainRenderPass;
		SharedPtr<VulkanFramebuffer> m_MainFrameBuffer;
		SharedPtr<PipeLine> m_Pipline;

		friend class VulkanSwapChain;
	};
}
