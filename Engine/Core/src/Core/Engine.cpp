#include "pch.h"
#include "Engine.h"

#include "Platform/Windows/WindowsWindow.h"

#include "Vulkan/GraphhicsPipeline.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Shader.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanRenderer.h"
#include "Renderer/Renderer.h"
#include "Utilities/Colors.h"
#include "Event/Input.h"
#include "Utilities/DeltaTime.h"
#include "Utilities/Timer.h"
#include "Random.h"
#include "Renderer/OrthographicCamera.h"
#include "glm/gtc/epsilon.hpp"
#include "Math/Math.h"
#include "Map/World.h"
#include "Event/Event.h"

namespace PAL
{
	Engine* Engine::s_Instance;

	Engine::Engine(/*specs*/)
		: m_ShouldCloseWindow(false)
		, m_EngineState(EngineStates::UpdateAndRender)
	{
		CORE_PROFILER_FUNC();

		s_Instance = this;

		WindowData data{};
		data.Width = 800;
		data.Height = 600;
		data.Title = "PL Engine";
		data.Vsync = false;
		data.Mode = WindowMode::Windowed;

		m_Window = MakeUnique<WindowsWindow>(data);
		m_Window->SetupEventCallback(BIND_FUN(this, Engine::OnEvent));
	}

	void Engine::Run()
	{
		CORE_PROFILER_FUNC();

		// init
		m_EventHandler.BindAction(EventType::ResizeWindow, this, &Engine::OnResizeWindow);
		m_EventHandler.BindAction(EventType::CloseWindow, this, &Engine::OnCloseWindow);
		m_EventHandler.BindAction(EventType::KeyPressed, this, &Engine::OnKeyPressed);
		m_EventHandler.BindAction(EventType::FrameBufferResize, this, &Engine::OnResizeFrameBuffer);

		m_Renderer = MakeShared<Renderer>();
		m_Renderer->Init(RenderAPITarget::Vulkan);

		m_World = MakeShared<World>(); // default map for now 
		m_World->BeginPlay(); //TODO: move later, should be called from editor when press play 

		EngineLoop();
	}

	void Engine::Stop()
	{
		CORE_PROFILER_FUNC();

		m_Renderer->WaitForIdle();
		m_Renderer->Shutdown();
		m_Window->Close();
	}

	void Engine::EngineLoop()
	{
		while (!m_ShouldCloseWindow)
		{
			CORE_PROFILER_FRAME("CPU-Frame");

			m_DeltaTime.Update();
			m_Window->PollEvents();

			switch (m_EngineState)
			{
				case PAL::EngineStates::Render:
				{
					m_World->OnRender(m_DeltaTime.GetSeconds(), m_Renderer);
					break;
				}
				case PAL::EngineStates::UpdateAndRender:
				{
					m_World->OnUpdate(m_DeltaTime.GetSeconds());
					m_World->OnRender(m_DeltaTime.GetSeconds(), m_Renderer);
					break;
				}
				case PAL::EngineStates::Idle:
				{
					break;
				}
			}
		}
	}

	void Engine::OnEvent(Event& e)
	{
		m_EventHandler.OnEvent(e);
	}

	void Engine::OnResizeWindow(const ResizeWindowEvent& event)
	{
		//Debug::Log("Resize Window: {}, {}", event.GetWidth(), event.GetHeight());

		m_Window->OnResize(event.GetWidth(), event.GetHeight());
	}

	void Engine::OnResizeFrameBuffer(const ResizeFrameBufferEvent& event)
	{
		//Debug::Log("Resize Framebuffer: {}, {}", event.GetWidth(), event.GetHeight());

		m_Renderer->ResizeFrameBuffer(true, event.GetWidth(), event.GetHeight());
	}

	void Engine::OnCloseWindow(const CloseWindowEvent& event)
	{
		m_ShouldCloseWindow = true;
	}

	void Engine::OnKeyPressed(const KeyPressedEvent& event)
	{
		if (event.GetPressedKey() == KeyCode::Escape)
		{
			if (m_Window->GetWindowMode() == WindowMode::FullScreen)
				m_Window->SetScreenMode(WindowMode::Windowed);
			else
				m_Window->SetScreenMode(WindowMode::FullScreen);
		}
	}
}
