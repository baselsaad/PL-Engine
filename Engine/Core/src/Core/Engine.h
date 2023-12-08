#pragma once
#include "Window.h"
#include "Event/EventHandler.h"

namespace PL_Engine
{
	class VulkanAPI;


	class Engine
	{
	public:
		Engine(/*specs*/);
		inline static Engine* Get() { return s_Instance; }

		virtual void Run();

		void SetupEventCallbacks();
		void OnResizeWindow(const ResizeWindowEvent& event);
		void OnCloseWindow(const CloseWindowEvent& event);

		inline const UniquePtr<Window>& GetWindow() const { return m_Window; }

	private:
		static Engine* s_Instance;

		UniquePtr<Window> m_Window;
		bool m_ShouldCloseWindow;

		EventHandler m_EventHandler;
		std::function<void(Event&)> m_EventCallback;
	};
}