#pragma once
#include "BatchRenderer.h"
#include "Camera.h"

namespace PL_Engine
{
	class IRenderAPI;
	class Camera;
	class EditorCamera;

	enum class RenderAPITarget
	{
		Vulkan, Unknown
	};

	struct RenderStats
	{
		int Quads = 0;
		int DrawCalls = 0;

		void Reset()
		{
			Quads = 0;
			DrawCalls = 0;
		}
	};

	class Renderer
	{
	public:
		static void Init(RenderAPITarget target);
		static void Shutdown();

		static void BeginFrame(const Camera& camera);
		static void EndFrame();
		
		static void DrawQuad(const glm::vec3& translation, const glm::vec3& scale ,const glm::vec3& color = glm::vec3(0.5f, 1.0f, 0.0f));// just for test

		static void SubmitCommand(const std::function<void()>& command);
		static void WaitForIdle();
		static void OnResizeWindow(bool resize = false, int width = 0, int height = 0);
	
	private:
		static void Flush();

	private:
		static SharedPtr<IRenderAPI> s_RenderAPI;
		static BatchRenderer* s_BatchRenderer;
		static glm::mat4 s_Projection;

		static RenderStats s_RenderStats;
	};
}