#pragma once

namespace PL_Engine
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class IRenderAPI
	{
	public:
		virtual ~IRenderAPI() {}

		virtual void Init() = 0;
		virtual void InitRenderApiContext() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void DrawQuad(SharedPtr<VulkanVertexBuffer> vertexBuffer, SharedPtr<VulkanIndexBuffer> indexBuffer, uint32_t indexCount, const glm::mat4& projection) = 0; // delete Later
		virtual void SubmitCommand(const std::function<void()>& command) = 0;

		virtual void WaitForIdle() = 0;
		virtual void OnResizeWindow(bool resize = false, int width = 0, int height = 0) = 0;
	};
}