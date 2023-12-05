#pragma once
#include "BatchRenderer.h"

namespace PL_Engine
{
	class IRenderAPI;

	enum class RenderAPITarget
	{
		Vulkan, Unknown
	};

	class Renderer
	{
	public:
		static void Init(RenderAPITarget target);
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

		static void Flush();
		static void DrawQuad(const glm::vec3& translation, const glm::vec3& scale ,const glm::vec3& color = glm::vec3(0.5f, 1.0f, 0.0f));// just for test

		static void SubmitCommand(const std::function<void()>& command);

		static void WaitForIdle();
		static void OnResizeWindow(bool resize = false, int width = 0, int height = 0);
	private:
		static SharedPtr<IRenderAPI> s_RenderAPI;
		static BatchRenderer* s_BatchRenderer;
	};
}