#pragma once

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

		static void DrawTriangle();// just for test

		static void WaitForIdle();
		static void OnResizeWindow(bool resize = false, int width = 0, int height = 0);

	private:
		static SharedPtr<IRenderAPI> s_RenderAPI;
	};
}