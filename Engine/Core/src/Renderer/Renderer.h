#pragma once
#include "BatchRenderer.h"
#include "Camera.h"

namespace PAL
{
	class IRenderAPI;
	class Camera;
	class EditorCamera;
	struct TransformComponent;

	enum class RenderAPITarget
	{
		Vulkan, Unknown
	};

	struct RenderStats
	{
		int Quads = 0;
		int DrawCalls = 0;
		int VertexBufferCount = 0;

		void Reset()
		{
			Quads = 0;
			DrawCalls = 0;
		}
	};

	class Renderer
	{
	public:
		void Init(RenderAPITarget target);
		void Shutdown();

		void StartFrame(const Camera& camera);
		void EndFrame();

		void DrawQuad(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& color = glm::vec3(0.5f, 1.0f, 0.0f));// just for test
		void DrawQuad(const TransformComponent& transform, const glm::vec3& color);// just for test

		void RecordCommand(const std::function<void()>& command);
		void WaitForIdle();
		void ResizeFrameBuffer(bool resize = false, int width = 0, int height = 0);

		static RenderStats& GetStats() { return s_RenderStats; }
	
	private:
		void Flush();

	private:
		SharedPtr<IRenderAPI> m_RenderAPI;
		BatchRenderer* m_BatchRenderer;
		glm::mat4 m_Projection; // remove later

		static RenderStats s_RenderStats;
	};
}