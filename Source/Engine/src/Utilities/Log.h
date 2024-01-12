#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"


class Debug
{
public:
	static void Init(const char* consoleName);

	template<typename...Args>
	static void Log(const Args&... args)
	{
		Debug::GetLogger()->debug(args ...);
	}

	template<typename...Args>
	static void Info(const Args&... args)
	{
		Debug::GetLogger()->info(args ...);
	}

	template<typename...Args>
	static void LogWarn(const Args&... args)
	{
		Debug::GetLogger()->warn(args ...);
	}

	template<typename...Args>
	static void LogError(const Args&... args)
	{
		Debug::GetLogger()->error(args ...);
	}

	template<typename...Args>
	static void Critical(const Args&... args)
	{
		Debug::GetLogger()->critical(args ...);
	}

private:
	static std::shared_ptr< spdlog::logger> s_Logger;
	inline static std::shared_ptr< spdlog::logger>& GetLogger() { return s_Logger; }
};

#define ASSERT_ERROR(...)		Debug::LogError(__VA_ARGS__);
#define ASSERT_WARNING(...)		Debug::LogWarn(__VA_ARGS__);