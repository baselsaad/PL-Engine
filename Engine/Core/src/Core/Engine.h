#pragma once
#include "Window.h"

namespace PL_Engine
{
	class VulkanAPI;


	class Engine
	{
	public:
		Engine(/*specs*/);
		inline static Engine* Get() { return s_Instance; }

		virtual void Run();

		inline const UniquePtr<Window>& GetWindow() const { return m_Window; }
	private:
		static Engine* s_Instance;
		UniquePtr<Window> m_Window;
	};
}