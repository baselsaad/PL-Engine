#include "pch.h"
#include "Log.h"


std::shared_ptr< spdlog::logger> Debug::s_Logger;

void Debug::Init(const char* consoleName)
{
	#if DEBUG
	spdlog::set_pattern("%^[%n] [%t] [%l]: %v%$");
	#else
	// (Date) (Thread) (LogLvel) [Message]
	spdlog::set_pattern("%^[%d-%m-%Y %S:%M:%H] [%n] [%t] [%l]: %v%$");
	#endif

	s_Logger = spdlog::stdout_color_mt(consoleName);
	s_Logger->set_level(spdlog::level::debug);
}

