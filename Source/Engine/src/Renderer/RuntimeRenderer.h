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

	struct RuntimeRendererSpecification
	{
		RenderAPITarget TargetAPI = RenderAPITarget::Vulkan;
		RenderApiSpec ApiSpec = {};

		glm::vec2 ViewportSize;
	};

	struct RenderStats
	{
		int Quads = 0;
		int DrawCalls = 0;
		int VertexBufferCount = 0;

		float FrameTime = 0.0f;
		float FrameTime_ms = 0.0f;
		uint32_t FramesPerSecond = 0;

	private:
		void Reset()
		{
			Quads = 0;
			DrawCalls = 0;
		}

		friend class Engine;
		friend class RuntimeRenderer;
		friend class Editor;
	};


	class RuntimeRenderer
	{
	public:
		void Init(const RuntimeRendererSpecification& spec);
		void Shutdown();

		void StartFrame();
		void EndFrame();
		void FlushDrawCommands();

		void DrawQuad(const glm::vec3& translation, const glm::vec3& scale, const glm::vec4& color = glm::vec4(0.5f, 1.0f, 0.0f,1.0f));// just for test
		void DrawQuad(const TransformComponent& transform, const glm::vec4& color);// just for test

		void SetProjection(const glm::mat4& projection) { m_Projection = projection; }

		void RecordDrawCommand(std::function<void()>&& command);
		void WaitForIdle();
		void ResizeFrameBuffer(bool resize = false, uint32_t width = 0, uint32_t height = 0);
		void* GetFinalImage(uint32_t index = 0);

		void SetVSync(bool vsync);

		static RenderStats& GetStats() { return s_RenderStats; }
		inline SharedPtr<IRenderAPI>& GetRenderAPI() { return m_RenderAPI; }

		inline const RuntimeRendererSpecification& GetRuntimeRendererSpec () const { return m_RuntimeRendererSpecification; }
	
	private:
		void Flush();

	private:
		static RenderStats s_RenderStats;

		SharedPtr<IRenderAPI> m_RenderAPI;
		BatchRenderer* m_BatchRenderer;

		RuntimeRendererSpecification m_RuntimeRendererSpecification;

		// remove later
		glm::mat4 m_Projection; 
	};
}