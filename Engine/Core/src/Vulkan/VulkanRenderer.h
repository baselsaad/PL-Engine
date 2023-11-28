#pragma once
#include "VulkanAPI.h"
#include "Renderer/RenderAPI.h"


namespace PL_Engine
{
	class CommandBuffer;
	class PipeLine;
	class RenderPass;

	class VulkanAPI : public IRenderAPI
	{
	public:
		void Init();
		virtual void InitRenderApiContext() override;
		virtual void Shutdown() override;
		virtual void DrawTriangle() override;
		virtual void WaitForIdle() override;
		virtual void OnResizeWindow(bool resize = false, int width = 0, int height = 0) override;

		inline static int GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
		inline const SharedPtr<RenderPass>& GetRenderPass() { return m_RenderPass; }
		inline const SharedPtr<PipeLine> GetGraphicsPipline() { return m_Pipline; }

	private:
		void DrawFrame();
		void CreateSyncObjects();

	private:
		SharedPtr<RenderPass> m_RenderPass;
		SharedPtr<PipeLine> m_Pipline;

		SharedPtr<CommandBuffer> m_CommandBuffer;

		// semaphores
		std::vector<VkSemaphore> m_ImageAvailableSemaphore;
		std::vector<VkSemaphore> m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFence;

		static const int MAX_FRAMES_IN_FLIGHT = 2;
		static uint32_t s_CurrentFrame;
		static bool s_ResizeFrameBuffer;
	};
}
