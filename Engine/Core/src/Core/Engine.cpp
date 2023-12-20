#include "pch.h"
#include "Engine.h"
#include "GLFW/glfw3.h"
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

namespace PAL
{
	Engine* Engine::s_Instance;

	Engine::Engine(/*specs*/)
		: m_ShouldCloseWindow(false)
	{
		s_Instance = this;

		WindowData data{};
		data.Width = 800;
		data.Height = 600;
		data.Title = "PL Engine";
		data.Vsync = false;

		m_Window = MakeUnique<WindowsWindow>(data);

		SetupEventCallbacks();
		m_EventHandler.BindAction(EventType::ResizeWindow, this, &Engine::OnResizeWindow);
		m_EventHandler.BindAction(EventType::CloseWindow, this, &Engine::OnCloseWindow);

		Renderer::Init(RenderAPITarget::Vulkan);
	}

	void Engine::EngineLoop()
	{
		World world;
		world.BeginPlay();

		while (!m_ShouldCloseWindow)
		{
			SCOPE_TIMER_NAME("Frame");

			m_DeltaTime.Update();
			m_Window->PollEvents();

			// TODO: GetWorld
			world.OnUpdate(m_DeltaTime.GetSeconds());
		}

		Renderer::WaitForIdle();
		Renderer::Shutdown();
		m_Window->Close();
	}

	void Engine::SetupEventCallbacks()
	{
		m_EventCallback = [this](Event& e) -> void { m_EventHandler.OnEvent(e); };
		glfwSetWindowUserPointer(*m_Window, &m_EventCallback);

		//@TODO: Move to the window 
		// Window Close 
		{
			auto callback = [](GLFWwindow* window)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window); //  eventCallback = m_EventCallback
				CloseWindowEvent e;
				eventCallback(e);
			};

			glfwSetWindowCloseCallback(*m_Window, callback);
		}

		// Window Resize
		{
			auto callback = [](GLFWwindow* window, int width, int height)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);
				ResizeWindowEvent event(width, height);
				eventCallback(event);
			};

			glfwSetWindowSizeCallback(*m_Window, callback);
			//glfwSetFramebufferSizeCallback(*m_Window, callback); later for editor
		}

		// Mouse Buttons
		{
			auto callback = [](GLFWwindow* window, int button, int action, int mods)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);
				double outX, outY;
				glfwGetCursorPos(window, &outX, &outY);

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button, outX, outY);
						eventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button, outX, outY);
						eventCallback(event);
						break;
					}
				}
			};

			glfwSetMouseButtonCallback(*m_Window, callback);
		}

		// Mouse Move
		{
			auto callback = [](GLFWwindow* window, double x, double y)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);

				MouseMoveEvent event((float)x, (float)y);
				eventCallback(event);
			};

			glfwSetCursorPosCallback(*m_Window, callback);
		}

		// Mouse Scroll
		{
			auto callback = [](GLFWwindow* window, double x, double y)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)x, (float)y);
				eventCallback(event);
			};

			glfwSetScrollCallback(*m_Window, callback);
		}

	}

	void Engine::OnResizeWindow(const ResizeWindowEvent& event)
	{
		Renderer::OnResizeWindow(true, event.GetWidth(), event.GetHeight());
		m_Window->OnResize(event.GetWidth(), event.GetHeight());
	}

	void Engine::OnCloseWindow(const CloseWindowEvent& event)
	{
		m_ShouldCloseWindow = true;
	}

}
