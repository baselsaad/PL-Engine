#pragma once
#include "Event.h"
#include "glm/vec2.hpp"

class Window;

class Input
{
public:
	static bool IsMouseButtonDown(const MouseButtonKey& key);
	static bool IsMouseButtonUp(const MouseButtonKey& key);
	static glm::vec2 GetMousePosition();
	
private:
	static void Init(Window* window);
	friend class Application;
};
