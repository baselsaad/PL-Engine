#pragma once
#include "Core/Window.h"
#include "Event/EventHandler.h"

namespace PAL
{
	class RuntimeRenderer;
	class World;
	struct EngineArgs;

	// must be inhirted by Editor, Standalone/Runtime App etc  
	class EngineApplication
	{
	public:
		virtual ~EngineApplication() {}


		virtual void Init() = 0;
		virtual void SetupInput(EventHandler& eventHandler) = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnShutdown(/*TODO: Shutdown reason*/) = 0;

		inline const SharedPtr<RuntimeRenderer>& GetRuntimeRenderer() { return m_RuntimeRenderer; };
		inline const SharedPtr<World>& GetCurrentWorld() { return m_CurrentWorld; };
		inline const glm::vec2& GetViewportSize() const { return m_ViewportSize; }

	protected:
		SharedPtr<RuntimeRenderer> m_RuntimeRenderer;
		SharedPtr<World> m_CurrentWorld; // maybe this should be part of engine class ? 
		glm::vec2 m_ViewportSize = { 0.0f,0.0f }; // this could be viewport in the editor or the whole window 
	};

	EngineArgs ApplicationMain(int argc, char** argv);
}