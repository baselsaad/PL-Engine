#pragma once

namespace PL_Engine
{
	class IRenderAPI 
	{
	public:
		virtual void Init() = 0;
		virtual void InitRenderApiContext() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void DrawQuad(const glm::vec3& translation = glm::vec3(0.0f)) = 0; // delete Later

		virtual void WaitForIdle() = 0;
		virtual void OnResizeWindow(bool resize = false, int width = 0, int height = 0) = 0;
	};
}