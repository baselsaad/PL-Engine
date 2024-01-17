#pragma once
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "Utilities/Log.h"

#if _MSC_VER
#define DEBUG_BREAK() __debugbreak();
#else
#define DEBUG_BREAK throw std::exception
#endif

#if DEBUG

#define PAL_ASSERT(x, ...)			        \
		if (!(x))					     	\
		{									\
			Debug::LogError(__VA_ARGS__);   \
			DEBUG_BREAK();		            \
		}

#define PAL_CHECK(x, ...)			        \
		if (!(x))				            \
		{						            \
			Debug::LogWarn(__VA_ARGS__);    \
		}

#define VK_CHECK_RESULT(f)													\
{																			\
	VkResult res = (f);														\
	if (res != VK_SUCCESS)													\
	{																		\
		Debug::LogError("Fail to Create {0} {1}", __FILE__, __LINE__);		\
		DEBUG_BREAK()														\
	}																		\
}
#else
#define PAL_ASSERT(x,...)
#define PAL_CHECK(x, ...)
#define VK_CHECK_RESULT(f)	(f)
#endif



