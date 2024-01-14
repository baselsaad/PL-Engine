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
#include "Renderer/RuntimeRenderer.h"
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
#include "Platform/PlatformEntry.h"
#include "Utilities/Debug.h"

namespace PAL
{
	Engine* Engine::s_Instance;

	Engine::Engine(const EngineArgs& engineArgs)
		: m_ShouldClose(false)
		, m_App(engineArgs.CurrentApp)
	{
		CORE_PROFILER_FUNC();

		s_Instance = this;

		m_Window = NewUnique<WindowsWindow>(engineArgs.EngineWindowData);
		m_Window->InitContext();
		m_Window->SetupEventCallback(BIND_FUN(this, Engine::OnEvent));

		m_App->Init();
	}

	void Engine::Run()
	{
		CORE_PROFILER_FUNC();

		// init
		m_EventHandler.BindAction(EventType::ResizeWindow, this, &Engine::OnResizeWindow);
		m_EventHandler.BindAction(EventType::CloseWindow, this, &Engine::OnCloseWindow);
		m_EventHandler.BindAction(EventType::KeyPressed, this, &Engine::OnKeyPressed);

		EngineLoop();
	}

	void Engine::Stop()
	{
		CORE_PROFILER_FUNC();

		m_App->OnShutdown();
		delete m_App;

		m_Window->Close();
	}

	void Engine::Exit()
	{
		m_ShouldClose = true;
	}

	void Engine::SetVSync(bool vsync)
	{
		if (m_Window->IsVsyncOn() != vsync)
		{
			m_App->GetRuntimeRenderer()->SetVSync(vsync);
			m_Window->SetVsync(vsync);
		}
	}

	const SharedPtr<RuntimeRenderer>& Engine::GetRuntimeRenderer()
	{
		return m_App->GetRuntimeRenderer();
	}

	const glm::vec2& Engine::GetViewportSize()
	{
		return m_App->GetViewportSize();
	}

	void Engine::EngineLoop()
	{
		// EngineLoop should tick: 
		// 1- m_DeltaTime
		// 2-Window(PollEvents)
		// 3-App(Editor, Runtime, etc)
		// 4-TimerManager(like ue)
		// 5-plugins -> PresentFrame 

		while (!m_ShouldClose)
		{
			CORE_PROFILER_FRAME("CPU-Frame");

			m_DeltaTime.Update();

			RuntimeRenderer::GetStats().Reset();
			RuntimeRenderer::GetStats().FrameTime = m_DeltaTime.GetAvgDeltaTimeInSeconds();
			RuntimeRenderer::GetStats().FrameTime_ms = m_DeltaTime.GetAvgDeltaTimeInMilliSeconds();
			RuntimeRenderer::GetStats().FramesPerSecond = m_DeltaTime.GetAverageFPS();

			m_Window->PollEvents();

			// Let the App start and end the frame 
			m_App->OnUpdate(m_DeltaTime.GetDeltaInSeconds());

			// Tick TimerManager

			// Tick Plugins etc.. 

			m_Window->Present();
		}
	}

	void Engine::OnEvent(Event& e)
	{
		m_EventHandler.OnEvent(e);
	}

	void Engine::OnResizeWindow(const ResizeWindowEvent& event)
	{
		m_Window->OnResize(event.GetWidth(), event.GetHeight());
	}

	void Engine::OnCloseWindow(const CloseWindowEvent& event)
	{
		m_ShouldClose = true;
	}

	void Engine::OnKeyPressed(const KeyPressedEvent& event)
	{
		if (event.GetPressedKey() == KeyCode::F11)
		{
			if (m_Window->GetWindowMode() == WindowMode::FullScreen)
				m_Window->SetScreenMode(WindowMode::Windowed);
			else
				m_Window->SetScreenMode(WindowMode::FullScreen);
		}
	}

}
