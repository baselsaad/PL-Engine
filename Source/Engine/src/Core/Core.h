#pragma once


// Macro to disable warnings for different compilers
#if defined(__GNUC__) // GCC or Clang
#define DISABLE_WARNING_PUSH _Pragma("GCC diagnostic push")
#define DISABLE_WARNING_POP _Pragma("GCC diagnostic pop")
#define DISABLE_WARNING(warningName) _Pragma("GCC diagnostic ignored \"" #warningName "\"")

#elif defined(_MSC_VER) // Visual Studio
#define DISABLE_WARNING_PUSH __pragma(warning(push))
#define DISABLE_WARNING_POP __pragma(warning(pop))
#define DISABLE_WARNING(...) __pragma(warning(disable: __VA_ARGS__))
#else
	// Compiler not recognized, define empty macros
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING(warningName)
#endif
