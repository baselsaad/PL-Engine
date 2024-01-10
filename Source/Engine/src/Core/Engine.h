#pragma once
#include "Window.h"
#include "Event/EventHandler.h"
#include "Utilities/DeltaTime.h"

namespace PAL
{
	class VulkanAPI;
	class Renderer;
	class World;
	class EngineArgs;

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

		inline const UniquePtr<Window>& GetWindow() const { return m_Window; }
		inline EventHandler& GetInputHandler() { return m_EventHandler; }
		inline SharedPtr<Renderer>& GetRenderer() { return m_Renderer; }

	private:
		virtual void EngineLoop();
		void OnEvent(Event& e);
		void OnResizeWindow(const ResizeWindowEvent& event);
		void OnResizeFrameBuffer(const ResizeFrameBufferEvent& event); // TODO: move later to editor
		void OnCloseWindow(const CloseWindowEvent& event);
		void OnKeyPressed(const KeyPressedEvent& event);

	private:
		static Engine* s_Instance;

		SharedPtr<Renderer> m_Renderer;
		SharedPtr<World> m_World;
		UniquePtr<Window> m_Window;

		DeltaTime m_DeltaTime;
		EngineStates m_EngineState;
		EventHandler m_EventHandler;

		bool m_ShouldCloseWindow;
	};
}