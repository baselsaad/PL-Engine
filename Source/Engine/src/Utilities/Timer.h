#pragma once
#include "InstrumentationTimer.h"


#define PROFILE_TO_JSON 0
#define PROFILE_TO_OPTICK 1

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


#if PROFILE_TO_JSON

#define BEGIN_PROFILE_SESSION(name)             Instrumentor::Get().BeginSession(name)
#define END_PROFILE_SESSION()                   Instrumentor::Get().EndSession()

#define CORE_PROFILER_TAG(TAG)                  InstrumentationTimer timer(TAG)
#define CORE_PROFILER_FUNC()                    CORE_PROFILER_TAG(FUNC_NAME)
#define CORE_PROFILER_SCOPE(TAG)                CORE_PROFILER_TAG(TAG)
#define CORE_PROFILER_SCOPE_DYNAMIC(TAG)        CORE_PROFILER_TAG(TAG)
#define CORE_PROFILER_FRAME(TAG)                CORE_PROFILER_TAG(TAG)

#elif PROFILE_TO_OPTICK
#include "optick.h"

#define BEGIN_PROFILE_SESSION(name)	
#define END_PROFILE_SESSION()		

#define CORE_PROFILER_TAG(TAG, ...)             OPTICK_TAG(TAG, __VA_ARGS__)
#define CORE_PROFILER_FUNC()                    OPTICK_EVENT()
#define CORE_PROFILER_SCOPE(TAG)                OPTICK_EVENT(TAG)
#define CORE_PROFILER_SCOPE_DYNAMIC(TAG)        OPTICK_EVENT_DYNAMIC(TAG)
#define CORE_PROFILER_FRAME(TAG)                OPTICK_FRAME(TAG)

#else 

#define BEGIN_PROFILE_SESSION(name)	
#define END_PROFILE_SESSION()		

#define CORE_PROFILER_TAG(TAG)			
#define CORE_PROFILER_FUNC()	
#define CORE_PROFILER_SCOPE(TAG)
#define CORE_PROFILER_SCOPE_DYNAMIC(TAG)    
#define CORE_PROFILER_FRAME(TAG)			
#endif

