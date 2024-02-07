#pragma once



#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <filesystem>


#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "Core/Core.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <deque>


#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

#include <chrono>
#include <random>
#include <thread>
#include <optional>


DISABLE_WARNING_PUSH
DISABLE_WARNING(4996 26819 26495 26800)
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
DISABLE_WARNING_POP

DISABLE_WARNING_PUSH
DISABLE_WARNING(26827)
#include <entt/entt.hpp>
DISABLE_WARNING_POP


#include <glm/glm.hpp>                   // Basic glm types: vec2, vec3, vec4, mat2, mat3, mat4
#include <glm/gtc/matrix_transform.hpp>  // Transformations: translate, rotate, scale, perspective, ortho, etc.
#include <glm/gtc/constants.hpp>         // Common mathematical constants
#include <glm/gtc/quaternion.hpp>        // Quaternions
#include <glm/gtx/quaternion.hpp>        // Quaternions
#include <glm/gtc/type_ptr.hpp>          // Conversion functions for glm types
#include <glm/gtc/epsilon.hpp>
#include "Math/Math.h"



#ifdef PLATFORM_WINDOWS
	#include <Windows.h>
#endif

//Engine
#include "Core/MemoryTracker.h"
#include "Core/Memory.h"
#include "Utilities/Debug.h"
#include "Utilities/Colors.h"

