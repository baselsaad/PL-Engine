#pragma once
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "Utilities/Log.h"

#if _MSC_VER
#define DEBUG_BREAK __debugbreak
#else
#define DEBUG_BREAK throw std::exception
#endif

#if DEBUG

#define ASSERT(x, Msg)			\
		if (!(x))				\
		{						\
			ASSERT_ERROR(Msg);	\
			DEBUG_BREAK();		\
		}

#define VK_CHECK_RESULT(f)													\
{																			\
	VkResult res = (f);														\
	if (res != VK_SUCCESS)													\
	{																		\
		Debug::LogError("Fail to Create {0} {1}", __FILE__, __LINE__);		\
		DEBUG_BREAK();														\
	}																		\
}
#else
#define ASSERT(x,MSG)
#define VK_CHECK_RESULT(f)	(f)
#endif



