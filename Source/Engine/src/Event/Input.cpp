#include "pch.h"
#include "Input.h"

#include "Core/Window.h"
#include "Core/Engine.h"
#include "GLFW/glfw3.h"


namespace PAL
{
	static Window* window;

	bool Input::IsKeyPressed(const KeyCode key)
	{
		const UniquePtr<Window>& window = Engine::Get()->GetWindow();
		PAL_ASSERT(window, "No valid window!!");

		auto state = glfwGetKey(*window, static_cast<int32_t>(key));
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonDown(const MouseButtonKey key)
	{
		const UniquePtr<Window>& window = Engine::Get()->GetWindow();
		PAL_ASSERT(window, "No valid window!!");

		auto state = glfwGetMouseButton(*window, (uint32_t)key);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonUp(const MouseButtonKey key)
	{
		const UniquePtr<Window>& window = Engine::Get()->GetWindow();
		PAL_ASSERT(window, "No valid window!!");

		auto state = glfwGetMouseButton(*window, (uint32_t)key);
		return state == GLFW_RELEASE;
	}

	glm::vec2 Input::GetMousePosition()
	{
		const UniquePtr<Window>& window = Engine::Get()->GetWindow();
		PAL_ASSERT(window, "No valid window!!");

		double xPos, yPos;
		glfwGetCursorPos(*window, &xPos, &yPos);

		return { (float)xPos, window->GetWindowHeight() - (float)yPos };
	}
}