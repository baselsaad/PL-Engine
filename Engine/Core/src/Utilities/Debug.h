#pragma once
#include <stdlib.h>
#include "Utilities/Log.h"
#include <GLFW/glfw3.h>

#if _MSC_VER
#define DEBUG_BREAK __debugbreak
#else
#define DEBUG_BREAK throw std::exception
#endif

#if DEBUG

#define ASSERT(x, Msg)								\
		if (!(x))									\
		{											\
			ASSERT_ERROR(Msg);						\
			DEBUG_BREAK();							\
		}
#else
#define ASSERT(x,MSG)
#endif

