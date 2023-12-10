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
	}

	static void BenchmarkBatchRenderer(OrthographicCamera& camera)
	{
		//float cord = camera.GetZoom() * 2.0f;
		float cord = 5.0f;
		const glm::vec3 scale(0.45f);

		for (float y = -cord; y < 5.0f; y += 0.5f)
		{
			for (float x = -cord; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				Renderer::DrawQuad({ x, y, 0.0f }, scale, color);
			}
		}
	}

	void Engine::Run()
	{
		Renderer::Init(RenderAPITarget::Vulkan);

		const glm::vec3 scale(1.0f);
		DeltaTime deltaTime;

		//test
		OrthographicCamera camera(m_Window->GetAspectRatio());
		camera.SetupInput(m_EventHandler);

		BEGIN_PROFILE_SESSION("RenderingLoop", "RenderingLoop.json");

		while (!m_ShouldCloseWindow)
		{
			SCOPE_TIMER_NAME("Frame");

			deltaTime.Update();
			m_Window->PollEvents();

			camera.OnUpdate(deltaTime.GetSeconds());
			
			Renderer::BeginFrame(camera);
			{
				BenchmarkBatchRenderer(camera);

				Renderer::DrawQuad(glm::vec3(0.5f, 1.0f, 0.0f), scale, Colors::Spring_Green);
				Renderer::DrawQuad(glm::vec3(0.0f, 0.0f, 0.0f), scale, Colors::Blue);
				Renderer::DrawQuad(glm::vec3(5.0f, 3.5f, 0.0f), scale, Colors::Dark_Magenta);
				Renderer::DrawQuad(glm::vec3(-4.0f, -3.0f, 0.0f), scale, Colors::Tan);
				Renderer::DrawQuad(glm::vec3(2.0f, -3.5f, 0.0f), scale, Colors::Yellow);
			}
			Renderer::EndFrame();
		}

		END_PROFILE_SESSION();

		Renderer::WaitForIdle();
		Renderer::Shutdown();
		m_Window->Close();
	}

	void Engine::SetupEventCallbacks()
	{
		m_EventCallback = [this](Event& e) -> void { m_EventHandler.OnEvent(e); };
		glfwSetWindowUserPointer(*m_Window, &m_EventCallback);

		// Window Close 
		{
			auto callback = [](GLFWwindow* window)
			{
				auto& eventCallback = *(EventFuncType*)glfwGetWindowUserPointer(window); //  eventCallback = m_EventCallback
				CloseWindowEvent e;
				eventCallback(e);
			};

			glfwSetWindowCloseCallback(*m_Window, callback);
		}

		// Window Resize
		{
			auto callback = [](GLFWwindow* window, int width, int height)
			{
				auto& eventCallback = *(EventFuncType*)glfwGetWindowUserPointer(window);
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
				auto& eventCallback = *(EventFuncType*)glfwGetWindowUserPointer(window);
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
				auto& eventCallback = *(EventFuncType*)glfwGetWindowUserPointer(window);

				MouseMoveEvent event((float)x, (float)y);
				eventCallback(event);
			};

			glfwSetCursorPosCallback(*m_Window, callback);
		}

		// Mouse Scroll
		{
			auto callback = [](GLFWwindow* window, double x, double y)
			{
				auto& eventCallback = *(EventFuncType*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)x, (float)y);
				eventCallback(event);
			};

			glfwSetScrollCallback(*m_Window, callback);
		}

	}

	void Engine::OnResizeWindow(const ResizeWindowEvent& event)
	{
		Renderer::OnResizeWindow(true, event.GetWidth(), event.GetHeight());
		Engine::Get()->GetWindow()->OnResize(event.GetWidth(), event.GetHeight());
	}

	void Engine::OnCloseWindow(const CloseWindowEvent& event)
	{
		m_ShouldCloseWindow = true;
	}

}
