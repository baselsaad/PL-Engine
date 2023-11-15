#include "pch.h"
#include "Window.h"
#include "Utilities/Debug.h"
#include "GLFW/glfw3.h"
#include <GL/gl.h>


Window::Window(const WindowData& data)
	: m_Width(data.Width)
	, m_Height(data.Height)
	, m_Vsync(true)
{
	int state = glfwInit();
	ASSERT(state == GLFW_TRUE, "GLFW cannot be initialized!");

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	m_Window = glfwCreateWindow(data.Width, data.Height, data.Title.c_str(), NULL, NULL);
	ASSERT(m_Window, "Window is null!");

	// Make the window's context current
	glfwMakeContextCurrent(m_Window);

	HandleErrorMessages();
	PrintGpuInformation();
	SetVsync(true);
}

Window::~Window()
{
	glfwTerminate();
}

void Window::Clear()
{
}

void Window::Swap()
{
	/* Swap front and back buffers */
	glfwSwapBuffers(m_Window);
}

void Window::PollEvents()
{
	/* Poll for and process events */
	glfwPollEvents();
}

void Window::SetVsync(bool enable)
{
	m_Vsync = enable;
	glfwSwapInterval(enable);
}

void Window::UpdateWindowSize(int width, int height)
{
	m_Width = width;
	m_Height = height;
}

static void GLFWErrorCallback(int error, const char* desc)
{
	Debug::LogError("Error {0}: {1}", error, desc);
}

void Window::HandleErrorMessages()
{
	glfwSetErrorCallback(GLFWErrorCallback);
}

void Window::PrintGpuInformation()
{
	// GPU driver and OpenGL Information
	//Debug::Info("------------OpenGL Info------------------------");
	//Debug::Info(">> GPU: {}", glGetString(GL_RENDERER));
	//Debug::Info(">> OpenGL-Version: {}", glGetString(GL_VERSION));
	//Debug::Info(">> GLSL version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
	Debug::Info("-----------------------------------------------");
}