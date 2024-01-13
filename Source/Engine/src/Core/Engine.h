#pragma once
#include "Window.h"
#include "Event/EventHandler.h"
#include "Utilities/DeltaTime.h"
#include "Platform/PlatformEntry.h"

namespace PAL
{
	class VulkanAPI;
	class RuntimeRenderer;
	class World;

	enum class EngineStates
	{
		Render, UpdateAndRender, Idle
	};

	class Engine
	{
	public:
		Engine() = default;
		Engine(const EngineArgs& engineArgs);
		
		inline static Engine* Get() { return s_Instance; }

		// Start the engine main loop and load resources we need 
		virtual void Run();
		// Stop and free 
		virtual void Stop();

		virtual void Exit();

		void SetVSync(bool vsync);

		const SharedPtr<RuntimeRenderer>& GetRuntimeRenderer();

		inline const UniquePtr<Window>& GetWindow() const { return m_Window; }
		inline EventHandler& GetInputHandler() { return m_EventHandler; }
		const glm::vec2& GetViewportSize();
	private:
		virtual void EngineLoop();
		void OnEvent(Event& e);
		void OnResizeWindow(const ResizeWindowEvent& event);
		void OnCloseWindow(const CloseWindowEvent& event);
		void OnKeyPressed(const KeyPressedEvent& event);

	private:
		static Engine* s_Instance;

		EngineApplication* m_App;
		UniquePtr<Window> m_Window;
		DeltaTime m_DeltaTime;
		EventHandler m_EventHandler;

		bool m_ShouldClose;
	};
}