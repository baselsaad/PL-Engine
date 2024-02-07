#include "pch.h"
#include "DeltaTime.h"
#include "GLFW/glfw3.h"

namespace PAL
{

	void DeltaTime::Update()
	{
		// DeltaTime
		float currentTime = (float)glfwGetTime();
		m_DeltaTime.FrameTime = currentTime - m_DeltaTime.LastFrameTime;
		m_DeltaTime.LastFrameTime = currentTime;

		// FPS
		m_AvgData.FrameCount++;
		m_AvgData.TotalFrameTimeInOneSec += m_DeltaTime.FrameTime;

		if (currentTime - m_AvgData.LastFrameTime >= 1.0)
		{
			m_AvgData.FPS = m_AvgData.FrameCount / (int)(currentTime - m_AvgData.LastFrameTime);
			m_AvgData.AvgFrameTime = m_AvgData.TotalFrameTimeInOneSec / m_AvgData.FPS;

			m_AvgData.LastFrameTime = currentTime;
			m_AvgData.FrameCount = 0;
			m_AvgData.TotalFrameTimeInOneSec = 0.0f;
		}
	}

}