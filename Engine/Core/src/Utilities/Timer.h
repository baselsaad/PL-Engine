#pragma once
#include "InstrumentationTimer.h"

class Timer
{

public:
	Timer(const char* name = "Unknown");
	~Timer();

public:
	void StartTimer();
	void StopTimer();

	inline float ElapsedTimeInMili() const { return m_ElapsedTime; }
	inline float ElapsedTimeInSecound() const { return m_ElapsedTime / 1000.0f; }

private:
	static const int INVALID = -1;
	mutable float m_ElapsedTime;
	mutable bool m_IsTimerStarted = false;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_End;
	std::chrono::duration<float> m_Duration;

	const char* m_Name;
};

/**
 * From: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Debug/Instrumentor.h
 */
#if defined (__FUNCSIG__)
#define FUNC_NAME __FUNCSIG__
#else
#define FUNC_NAME "__FUNCSIG__ not supported in your Compiler"
#endif

#if 0
#define BEGIN_PROFILE_SESSION(name)	Instrumentor::Get().BeginSession(name)
#define END_PROFILE_SESSION()		Instrumentor::Get().EndSession()

#define SCOPE_TIMER_NAME(name) InstrumentationTimer timer(name)
#define SCOPE_TIMER() SCOPE_TIMER_NAME(FUNC_NAME)

#else 
#define BEGIN_PROFILE_SESSION(name)	
#define END_PROFILE_SESSION()	

#define SCOPE_TIMER_NAME(name)
#define SCOPE_TIMER()
#endif

