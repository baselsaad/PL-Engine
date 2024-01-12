#include "pch.h"
#include "DeltaTime.h"
#include "GLFW/glfw3.h"

namespace PAL
{

	void DeltaTime::Update()
	{
		// DeltaTime
		double now = glfwGetTime();
		m_FrameTime = (now - m_LastFrameTime);
		m_LastFrameTime = now;

		m_TotalFrameTime -= m_FrameTimeArray[m_FrameTimeIndex];
		m_FrameTimeArray[m_FrameTimeIndex] = m_FrameTime;
		m_TotalFrameTime += m_FrameTime;

		m_FrameTimeIndex = (m_FrameTimeIndex + 1) % m_MaxSamples;

		CalculateAverage();
	}

	void DeltaTime::CalculateAverage()
	{
		m_AverageFrameTime = m_TotalFrameTime / static_cast<double>(m_MaxSamples);
	}
}