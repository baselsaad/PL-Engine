#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{
	class Shader
	{
	public:
		static std::vector<char> ReadFile(const std::string& filename);
		static VkShaderModule CreateShaderModule(const std::vector<char>& code);
	};
}