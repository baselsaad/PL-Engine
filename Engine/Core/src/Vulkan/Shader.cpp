#include "pch.h"
#include "Shader.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

namespace PL_Engine
{
	std::vector<char> Shader::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		//std::filesystem::path filePath = std::filesystem::absolute(filename);
		//std::cout << "Attempting to open file: " << filePath << std::endl;

		if (!file.is_open())
		{
			ASSERT(false, "failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule Shader::CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::GetVulkanDevice()->GetVkDevice(), &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}
}
