#include "Shader.h"

#include "Utilities/Debug.h"
#include "vulkan/vulkan_core.h"

#include <iostream>
#include <fstream>
#include <filesystem>

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


