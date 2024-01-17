#pragma once
#include "Window.h"
#include "Event/EventHandler.h"
#include "Utilities/DeltaTime.h"

namespace PAL
{
	class VulkanAPI;
	class RuntimeRenderer;
	class EngineApplication;
	class World;
	enum class RenderAPITarget;

	struct EngineArgs
	{
		int ArgumentsCount;
		char** RawArgumentStrings;

		EngineApplication* CurrentApp;
		std::string AppName; //Debug
		WindowData EngineWindowData;
	};

	enum class EngineStates
	{
		Render, UpdateAndRender, Idle
	};

	class Engine
	{
	public:
		Engine() = delete;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;

		Engine(const EngineArgs& engineArgs);
		
		inline static Engine* Get() { return s_Instance; }

		// Start the engine main loop and load resources we need 
		virtual void Run();
		// Stop MainLoop
		virtual void Stop();
		// Stop and free 
		virtual void Exit();

		void SetVSync(bool vsync);

		// Window&Viewport
		const glm::vec2& GetViewportSize();
		inline const UniquePtr<Window>& GetWindow() const { return m_Window; }

		// Event
		inline EventHandler& GetInputHandler() { return m_EventHandler; }

		// DeltaTime
		inline float GetDeltaTime() const { return m_DeltaTime.GetDeltaInSeconds(); }

		// Runtime renderer
		const SharedPtr<RuntimeRenderer>& GetRuntimeRenderer();
		RenderAPITarget GetCurrentRenderAPI();  
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