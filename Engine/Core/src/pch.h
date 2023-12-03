#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <chrono>
#include <thread>
#include <optional>


#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#ifdef PLATFORM_WINDOWS
	#include <Windows.h>
#endif


#include "Core/Memory.h"
#include "Utilities/Debug.h"
