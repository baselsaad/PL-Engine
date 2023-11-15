#pragma once


class Timer
{

public:
	Timer(const char* name = "Unknown");
	~Timer();

public:
	void StartTimer() const;
	void StopTimer() const;

	inline float ElapsedTimeInMili() const { return m_ElapsedTime; }
	inline float ElapsedTimeInSecound() const { return m_ElapsedTime / 1000.0f; }

private:
	static const int INVALID = -1;
	mutable float m_ElapsedTime;
	mutable bool m_IsTimerStarted = false;

	mutable std::chrono::time_point<std::chrono::steady_clock> m_Start;
	mutable std::chrono::time_point<std::chrono::steady_clock> m_End;
	mutable std::chrono::duration<float> m_Duration;

	const char* m_Name;
};

/**
 * From: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Debug/Instrumentor.h
 */
#if defined (__FUNCTION__)
#define FUNC_NAME __FUNCTION__
#else
#define FUNC_NAME "__FUNCTION__ not supported in your Compiler"
#endif

#if (DEBUG)
#define SCOPE_TIMER_NAME(name) Timer timer(name)
#define SCOPE_TIMER() SCOPE_TIMER_NAME(FUNC_NAME)
#else 
#define SCOPE_TIMER_NAME(name)
#define SCOPE_TIMER()
#endif