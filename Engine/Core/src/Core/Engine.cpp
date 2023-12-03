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

namespace PL_Engine
{
	Engine* Engine::s_Instance;

	Engine::Engine(/*specs*/)
	{
		s_Instance = this;

		WindowData data{};
		data.Width = 800;
		data.Height = 600;
		data.Title = "PL Engine";
		data.Vsync = false;

		m_Window = MakeUnique<WindowsWindow>(data);

		glfwSetWindowUserPointer(*m_Window, m_Window.get());

		glfwSetFramebufferSizeCallback(*m_Window, [](GLFWwindow* window, int width, int height) -> void
		{
			Renderer::OnResizeWindow(true, width, height);
		});
	}

	void Engine::Run()
	{
		Renderer::Init(RenderAPITarget::Vulkan);

		while (!m_Window->ShouldClose())
		{
			m_Window->PollEvents();

			Renderer::BeginFrame();
			{
				Renderer::DrawQuad(glm::vec3(0.5f, -0.2f, 0.0f));
				Renderer::DrawQuad(glm::vec3(-0.4f, 0.2f, 0.0f));
				Renderer::DrawQuad(glm::vec3(-0.2f, 0.9f, 0.0f));
			}
			Renderer::EndFrame();
		}

		Renderer::WaitForIdle();

		Renderer::Shutdown();
		m_Window->Close();
	}
}
