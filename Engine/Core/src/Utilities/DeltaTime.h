#pragma once

class DeltaTime
{
public:
	DeltaTime() = default;
public:
	void Update();

	inline float GetMilliSeconds() const { return m_FrameTime * 1000.0f; }
	inline float GetSeconds() const { return m_FrameTime; }
	inline int GetFramePerSecounds() const { return (int)(1.0f / m_FrameTime); }

private:
	double m_LastFrameTime = 0.0f;
	float m_FrameTime = 0.0f;
};