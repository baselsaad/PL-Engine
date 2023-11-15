#include <iostream>

#include "Utilities/Debug.h"
#include "Core/Window.h"

int main()
{
	Debug::Init("Engine-Editor");

	Debug::LogWarn("Hello World");

	WindowData data;
	data.Height = 800.0f;
	data.Width = 800.0f;
	data.Title = "EditorWindow";

	Window* editorWindow = new Window(data);

	while (!glfwWindowShouldClose(*editorWindow))
	{
		glfwSwapBuffers(*editorWindow);
		glfwPollEvents();
	}

}