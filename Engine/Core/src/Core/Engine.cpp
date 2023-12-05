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
		data.Vsync = true;

		m_Window = MakeUnique<WindowsWindow>(data);

		glfwSetWindowUserPointer(*m_Window, m_Window.get());

		glfwSetFramebufferSizeCallback(*m_Window, [](GLFWwindow* window, int width, int height) -> void
		{
			Renderer::OnResizeWindow(true, width, height);
			Engine::Get()->GetWindow()->OnResize(width, height);
		});
	}

	void Engine::Run()
	{
		Renderer::Init(RenderAPITarget::Vulkan);

		constexpr glm::vec3 scale(3.0f);
		std::srand(static_cast<unsigned int>(std::time(0)));

		while (!m_Window->ShouldClose())
		{
			m_Window->PollEvents();

			Renderer::BeginFrame();
			{
				Renderer::DrawQuad(glm::vec3(-2.0f, 1.0f, 0.0f), scale, Colors::Blue);
				Renderer::DrawQuad(glm::vec3(2.0f, -3.5f, 0.0f), scale, Colors::Yellow);
				Renderer::DrawQuad(glm::vec3(3.0f, 1.0f, 0.0f), scale, Colors::Spring_Green);
				Renderer::DrawQuad(glm::vec3(-4.0f, -3.0f, 0.0f), scale, Colors::Tan);
				Renderer::DrawQuad(glm::vec3(5.0f, 3.5f, 0.0f), scale, Colors::Dark_Magenta);
			}
			Renderer::EndFrame();
		}

		Renderer::WaitForIdle();

		Renderer::Shutdown();
		m_Window->Close();
	}
}
