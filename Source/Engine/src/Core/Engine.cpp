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
#include "Platform/PlatformEntry.h"

// Testing
#include "Vulkan/CommandBuffer.h"
#include "Utilities/Debug.h"
#include "../../Editor/src/Editor.h"
#include "Vulkan/VulkanFramebuffer.h"

namespace PAL
{
	Engine* Engine::s_Instance;

	Engine::Engine(const EngineArgs& engineArgs)
		: m_ShouldClose(false)
		, m_EngineState(EngineStates::UpdateAndRender)
	{
		CORE_PROFILER_FUNC();

		s_Instance = this;

		m_Window = NewUnique<WindowsWindow>(engineArgs.EngineWindowData);
		m_Window->SetupEventCallback(BIND_FUN(this, Engine::OnEvent));

		m_Renderer = NewShared<Renderer>();
		m_Renderer->Init(RenderAPITarget::Vulkan);

		// init editor
		Editor::GetInstance();
	}

	void Engine::Run()
	{
		CORE_PROFILER_FUNC();

		// init
		m_EventHandler.BindAction(EventType::ResizeWindow, this, &Engine::OnResizeWindow);
		m_EventHandler.BindAction(EventType::CloseWindow, this, &Engine::OnCloseWindow);
		m_EventHandler.BindAction(EventType::KeyPressed, this, &Engine::OnKeyPressed);

		m_World = NewShared<World>(); // default map for now 
		m_World->BeginPlay(); //TODO: move later, should be called from editor when press play 

		EngineLoop();
	}

	void Engine::Stop()
	{
		CORE_PROFILER_FUNC();

		m_Renderer->WaitForIdle();
		Editor::GetInstance().Shutdown();

		m_Renderer->Shutdown();
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
			m_Renderer->SetVSync(vsync);
			m_Window->SetVsync(vsync);
		}
	}

	void Engine::EngineLoop()
	{
		while (!m_ShouldClose)
		{
			CORE_PROFILER_FRAME("CPU-Frame");

			m_DeltaTime.Update();
			Renderer::GetStats().FrameTime = m_DeltaTime.GetAvgDeltaTimeInSeconds();
			Renderer::GetStats().FrameTime_ms = m_DeltaTime.GetAvgDeltaTimeInMilliSeconds();
			Renderer::GetStats().FramesPerSecond = m_DeltaTime.GetAverageFPS();

			m_Window->PollEvents();

			// Move to Scene/Runtime Renderer
			if (!m_Renderer->GetRenderAPI().As<VulkanAPI>()->GetSceneFrameBuffer()->GetSpecification().IsSwapchainTarget)
			{
				m_RuntimeViewportSize = { Editor::GetInstance().GetViewportSize().x , Editor::GetInstance().GetViewportSize().y };
				m_Renderer->ResizeFrameBuffer(false, (uint32_t)m_RuntimeViewportSize.x, (uint32_t)m_RuntimeViewportSize.y);
			}
			else
			{
				m_RuntimeViewportSize = { m_Window->GetWindowWidth() , m_Window->GetWindowHeight() };
			}

			// Start recording main command buffer
			m_Renderer->StartFrame();

			switch (m_EngineState)
			{
				case EngineStates::Render:
				{
					m_World->OnRender(m_DeltaTime.GetDeltaInSeconds());
					break;
				}
				case EngineStates::UpdateAndRender:
				{
					m_World->OnUpdate(m_DeltaTime.GetDeltaInSeconds());
					m_World->OnRender(m_DeltaTime.GetDeltaInSeconds());
					break;
				}
				case EngineStates::Idle:
				{
					break;
				}
			}


			// Render the world
			m_Renderer->FlushDrawCommands();

			// Render ImGui on top of everything
			Editor::GetInstance().BeginFrame();
			Editor::GetInstance().OnRenderImGui(m_Renderer->GetRenderAPI().As<VulkanAPI>()->GetSceneFrameBuffer()->GetFrameBufferImage());
			Editor::GetInstance().EndFrame();

			// End recording main command buffer
			m_Renderer->EndFrame();

			// Submit main command buffer and present image to swapchain
			m_Renderer->PresentFrame();
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
		m_ShouldClose = true;
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
