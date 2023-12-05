#include "pch.h"
#include "DeltaTime.h"
#include "GLFW/glfw3.h"


void DeltaTime::Update()
{
	// DeltaTime
	float now = glfwGetTime();
	m_FrameTime = (float)(0.0f - m_LastFrameTime);
	m_LastFrameTime = 0.0f;
}

