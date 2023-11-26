#include "pch.h"
#include "WindowsWindow.h"


namespace PL_Engine
{

	static void GLFWErrorCallback(int error, const char* desc)
	{
		Debug::LogError("Error {0}: {1}", error, desc);
	}

	WindowsWindow::WindowsWindow(const WindowData& data)
		: m_Window(nullptr), m_WindowData(data)
	{

		int state = glfwInit();
		ASSERT(state == GLFW_TRUE, "GLFW cannot be initialized!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(data.Width, data.Height, data.Title.c_str(), nullptr, nullptr);
		ASSERT(m_Window, "Window is null!");
		SetVsync(data.Vsync);

		// Make the window's context current
		glfwMakeContextCurrent(m_Window);
		HandleErrorMessages();
	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void WindowsWindow::SwapBuffers()
	{
		/* Swap front and back buffers */
		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::PollEvents()
	{
		/* Poll for and process events */
		glfwPollEvents();
	}

	void WindowsWindow::SetVsync(bool enable)
	{
		m_WindowData.Vsync = enable;
		glfwSwapInterval(enable);
	}

	void WindowsWindow::UpdateWindowSize(int width, int height)
	{
		m_WindowData.Width = width;
		m_WindowData.Height = height;
	}

	GLFWwindow* WindowsWindow::GetWindowHandle()
	{
		return m_Window;
	}

	const GLFWwindow* WindowsWindow::GetWindowHandle() const
	{
		return m_Window;
	}

	bool WindowsWindow::IsVsyncOn() const
	{
		return m_WindowData.Vsync;
	}

	uint32_t WindowsWindow::GetWindowWidth() const
	{
		return m_WindowData.Width;
	}

	uint32_t WindowsWindow::GetWindowHeight() const
	{
		return m_WindowData.Height;
	}

	float WindowsWindow::GetAspectRatio() const
	{
		return (float)m_WindowData.Width / (float)m_WindowData.Height;
	}

	void WindowsWindow::HandleErrorMessages()
	{
		glfwSetErrorCallback(GLFWErrorCallback);
	}
}