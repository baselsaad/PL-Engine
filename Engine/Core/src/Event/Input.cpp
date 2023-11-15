#include "pch.h"
#include "Input.h"
#include "Core/Window.h"
#include "Utilities/Debug.h"

static Window* s_Window;

void Input::Init(Window* window)
{
	ASSERT(s_Window == nullptr, "Init was called before!!!");

	s_Window = window;
}

bool Input::IsMouseButtonDown(const MouseButtonKey& key)
{
	ASSERT(s_Window, "No valid window!!");

	auto state = glfwGetMouseButton(*s_Window, (int)key);
	return state == GLFW_PRESS;
}

bool Input::IsMouseButtonUp(const MouseButtonKey& key)
{
	ASSERT(s_Window, "No valid window!!");

	auto state = glfwGetMouseButton(*s_Window, (int)key);
	return state == GLFW_RELEASE;
}

glm::vec2 Input::GetMousePosition()
{
	ASSERT(s_Window, "No valid window!!");

	double xPos, yPos;
	glfwGetCursorPos(*s_Window, &xPos, &yPos);

	return { (float)xPos, s_Window->GetWindowHeight() - (float)yPos };
}
