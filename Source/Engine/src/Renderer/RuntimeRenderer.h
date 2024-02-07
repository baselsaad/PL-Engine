#pragma once
#include "RenderAPI.h"
#include "BatchRenderer.h"
#include "Camera.h"
#include "RenderingCommands.h"

namespace PAL
{
	class RenderAPI;
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
		size_t Quads = 0;
		size_t DrawCalls = 0;
		size_t VertexBufferCount = 0;
		size_t DrawCommandsQueueUsage = 0;
		

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

		void DrawQuad(const TransformComponent& transform, const glm::vec4& color, uint32_t entityID = -1);// just for test

		void SetProjection(const glm::mat4& projection) { m_Projection = projection; }

		template<typename FuncType>
		void RecordDrawCommand(FuncType&& command)
		{
			m_DrawCommandsQueue.Record(command);
		}

		void WaitForIdle();
		void ResizeFrameBuffer(uint32_t width = 0, uint32_t height = 0, int frameIndex = -1);
		void* GetFinalImage(uint32_t index = 0);

		void SetVSync(bool vsync);

		inline static RenderStats& GetStats() { return s_RenderStats; }
		inline SharedPtr<RenderAPI>& GetRenderAPI() { return m_RenderAPI; }
		inline const RuntimeRendererSpecification& GetRuntimeRendererSpec () const { return m_RuntimeRendererSpecification; }
	
	private:
		void DrawBatch();
		void DrawQuad(const glm::mat4 transformationMatrix, const glm::vec4& color, uint32_t entityID);

	private:
		static RenderStats s_RenderStats;

		SharedPtr<RenderAPI> m_RenderAPI;
		BatchRenderer* m_BatchRenderer = nullptr;
		CommandsQueue m_DrawCommandsQueue;

		RuntimeRendererSpecification m_RuntimeRendererSpecification;

		// remove later
		glm::mat4 m_Projection; 
	};
}