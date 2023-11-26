#pragma once
#include "Window.h"
struct GLFWwindow;

namespace PL_Engine
{

	class App
	{
	public:
		App(/*specs*/);

		inline static App* Get() { return s_Instance; }
		static bool s_FramebufferResized;

		inline const SharedPtr<Window>& GetWindow() const { return m_Window; }
	private:
		static App* s_Instance;
		SharedPtr<Window> m_Window;
	};
}