#pragma once
#include "Event.h"
#include "glm/vec2.hpp"


namespace PL_Engine
{
	class Input
	{
	public:
		static bool IsKeyPressed(const KeyCode key);
		static bool IsMouseButtonDown(const MouseButtonKey key);
		static bool IsMouseButtonUp(const MouseButtonKey key);
		static glm::vec2 GetMousePosition();
	};

}