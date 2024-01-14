#include "pch.h"
#include "WindowsWindow.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanContext.h"
#include <type_traits>
#include "Event/Event.h"
#include "Event/EventHandler.h"


namespace PAL
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
		SetScreenMode(data.Mode, data.Width, data.Height);

		ASSERT(m_WindowHandle, "Window is null!");
		SetVsync(data.Vsync);

		// Make the window's context current
		glfwMakeContextCurrent(m_WindowHandle);
		HandleErrorMessages();

		Debug::Log("Window Created width: {}, height: {}", GetWindowWidth(), GetWindowHeight());
	}

	WindowsWindow::~WindowsWindow()
	{
	}

	void WindowsWindow::InitContext()
	{
		// it is ok to use vulkan-releated classes here since this class is only for windows Window
		VulkanContext::Init(m_WindowHandle);
		m_SwapChain = NewShared<VulkanSwapChain>(VulkanContext::GetVulkanDevice());
		m_SwapChain->Create();
	}

	void WindowsWindow::Present()
	{
		/* Swap front and back buffers */
		m_SwapChain->PresentFrame();
	}

	void WindowsWindow::PollEvents()
	{
		/* Poll for and process events */
		glfwPollEvents();
	}

	void WindowsWindow::SetVsync(bool enable)
	{
		m_WindowData.Vsync = enable;
		glfwSwapInterval(enable ? 1 : 0);
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

	const SharedPtr<VulkanSwapChain>& WindowsWindow::GetSwapChain() const
	{
		return m_SwapChain;
	}

	void WindowsWindow::SetupEventCallback(EventFunc&& callback)
	{
		m_EventCallback = callback;
		glfwSetWindowUserPointer(m_WindowHandle, &m_EventCallback);

		//@TODO: Move to the window 
		// Window Close 
		{
			auto callback = [](GLFWwindow* window)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window); // eventCallback = m_EventCallback
				CloseWindowEvent e;
				eventCallback(e);
			};

			glfwSetWindowCloseCallback(m_WindowHandle, callback);
		}

		// Window Resize
		{
			auto callback = [](GLFWwindow* window, int width, int height)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);
				ResizeWindowEvent event(width, height);
				eventCallback(event);
			};

			glfwSetWindowSizeCallback(m_WindowHandle, callback);
			glfwSetFramebufferSizeCallback(m_WindowHandle, callback); // later for editor
		}

		// FrameBuffer Resize
		{
			auto callback = [](GLFWwindow* window, int width, int height)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);
				ResizeFrameBufferEvent event(width, height);
				eventCallback(event);
			};

			glfwSetFramebufferSizeCallback(m_WindowHandle, callback); // later for editor
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

			glfwSetMouseButtonCallback(m_WindowHandle, callback);
		}

		// Mouse Move
		{
			auto callback = [](GLFWwindow* window, double x, double y)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);

				MouseMoveEvent event((float)x, (float)y);
				eventCallback(event);
			};

			glfwSetCursorPosCallback(m_WindowHandle, callback);
		}

		// Mouse Scroll
		{
			auto callback = [](GLFWwindow* window, double x, double y)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)x, (float)y);
				eventCallback(event);
			};

			glfwSetScrollCallback(m_WindowHandle, callback);
		}

		// KeyPressed
		{
			auto callback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto& eventCallback = *(EventCallback*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyReleaseEvent event((KeyCode)key);
						eventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyPressedEvent event((KeyCode)key);
						eventCallback(event);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyRepeatEvent event((KeyCode)key);
						eventCallback(event);
						break;
					}
				}
			};

			glfwSetKeyCallback(m_WindowHandle, callback);
		}
	}

	void WindowsWindow::SetScreenMode(WindowMode mode, uint32_t width /* = 1600*/, uint32_t height /* = 900*/)
	{
		if (mode == WindowMode::Windowed)
		{
			m_WindowData.Width = width;
			m_WindowData.Height = height;
			m_WindowData.Mode = WindowMode::Windowed;

			const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			// center the window
			int xPos = (videoMode->width - m_WindowData.Width) / 2;
			int yPos = (videoMode->height - m_WindowData.Height) / 2;

			glfwSetWindowMonitor(m_WindowHandle, nullptr, xPos, yPos, m_WindowData.Width, m_WindowData.Height, GLFW_DONT_CARE);
			
			Debug::Log("Change window to Windowed mode: width: {}, height: {}", m_WindowData.Width, m_WindowData.Height);
		}
		else
		{
			GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
			glfwSetWindowMonitor(m_WindowHandle, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			
			m_WindowData.Width = mode->width;
			m_WindowData.Height = mode->height;
			m_WindowData.Mode = WindowMode::FullScreen;

			Debug::Log("Change window to FullScreen mode: width: {}, height : {}", m_WindowData.Width, m_WindowData.Height);
		}
	}

	void WindowsWindow::Close()
	{
		m_SwapChain->CleanupSwapChain();
		VulkanContext::Shutdown();

		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void WindowsWindow::HandleErrorMessages()
	{

	}
}