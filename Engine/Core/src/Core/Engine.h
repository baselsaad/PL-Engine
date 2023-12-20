#pragma once
#include "Window.h"
#include "Event/EventHandler.h"
#include "Utilities/DeltaTime.h"

namespace PAL
{
	class VulkanAPI;


	class Engine
	{
	public:
		Engine(/*specs*/);
		inline static Engine* Get() { return s_Instance; }

		virtual void EngineLoop();

		void SetupEventCallbacks();
		void OnResizeWindow(const ResizeWindowEvent& event);
		void OnCloseWindow(const CloseWindowEvent& event);

		inline const UniquePtr<Window>& GetWindow() const { return m_Window; }
		inline EventHandler& GetInputHandler() { return m_EventHandler; }

	private:
		DeltaTime m_DeltaTime;

		static Engine* s_Instance;

		UniquePtr<Window> m_Window;
		bool m_ShouldCloseWindow;

		EventHandler m_EventHandler;
		std::function<void(Event&)> m_EventCallback;
	};
}