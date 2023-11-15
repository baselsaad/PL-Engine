#include "pch.h"
#include "Timer.h"
#include "Debug.h"

Timer::Timer(const char* name)
	: m_Name(name)
{
	StartTimer();
}

Timer::~Timer()
{
	StopTimer();
	Debug::Info("{0}: {1} ms", m_Name, ElapsedTimeInMili());
}

void Timer::StartTimer() const
{
	m_Start = std::chrono::high_resolution_clock::now();
	m_IsTimerStarted = true;
}

void Timer::StopTimer() const
{
	m_End = std::chrono::high_resolution_clock::now();
	ASSERT(m_IsTimerStarted, "You have to start the timer first!");

	m_Duration = m_End - m_Start;
	m_ElapsedTime = m_Duration.count() * 1000.0f;
	m_IsTimerStarted = false;
}

