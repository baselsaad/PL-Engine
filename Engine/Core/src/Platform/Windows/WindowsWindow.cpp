#include "pch.h"
#include "WindowsWindow.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanContext.h"
#include <type_traits>


namespace PL_Engine
{

	static void GLFWErrorCallback(int error, const char* desc)
	{
		Debug::LogError("Error {0}: {1}", error, desc);
	}

	WindowsWindow::WindowsWindow(const WindowData& data)
		: m_WindowHandle(nullptr), m_WindowData(data)
	{

		int state = glfwInit();
		ASSERT(state == GLFW_TRUE, "GLFW cannot be initialized!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_WindowHandle = glfwCreateWindow(data.Width, data.Height, data.Title.c_str(), nullptr, nullptr);
		ASSERT(m_WindowHandle, "Window is null!");
		SetVsync(data.Vsync);

		// Make the window's context current
		glfwMakeContextCurrent(m_WindowHandle);
		HandleErrorMessages();
	}

	WindowsWindow::~WindowsWindow()
	{
	}

	void WindowsWindow::SwapBuffers()
	{
		/* Swap front and back buffers */
	}

	void WindowsWindow::PollEvents()
	{
		/* Poll for and process events */
		glfwPollEvents();
	}

	void WindowsWindow::SetVsync(bool enable)
	{
		m_WindowData.Vsync = enable;
	}

	void WindowsWindow::OnResize(int width, int height)
	{
		m_WindowData.Width = width;
		m_WindowData.Height = height;
	}

	GLFWwindow* WindowsWindow::GetWindowHandle()
	{
		return m_WindowHandle;
	}

	const GLFWwindow* WindowsWindow::GetWindowHandle() const
	{
		return m_WindowHandle;
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

	void WindowsWindow::WaitEvents()
	{
		glfwWaitEvents();
	}

	void WindowsWindow::GetFramebufferSize(int& width, int& height)
	{
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);
	}

	bool WindowsWindow::ShouldClose()
	{
		return glfwWindowShouldClose(m_WindowHandle);
	}

	void WindowsWindow::Close()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void WindowsWindow::HandleErrorMessages()
	{

	}
}