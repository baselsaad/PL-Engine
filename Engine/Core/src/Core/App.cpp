#include "pch.h"
#include "App.h"
#include "GLFW/glfw3.h"
#include "Vulkan/VulkanContext.h"
#include "Platform/Windows/WindowsWindow.h"

namespace PL_Engine
{
	App* App::s_Instance;
	bool App::s_FramebufferResized;

	App::App(/*specs*/)
	{
		s_Instance = this;

		WindowData data{};
		data.Width = 800;
		data.Height = 600;
		data.Title = "PL Engine";
		data.Vsync = false;

		m_Window = MakeShared<WindowsWindow>(data);
		
		//glfwSetWindowUserPointer(s_Window, nullptr);
		//TODO: delete this later (only for tests now)
		glfwSetFramebufferSizeCallback(*m_Window, [](GLFWwindow* window, int width, int height)
		{
			s_FramebufferResized = true;
		});
	}

}
