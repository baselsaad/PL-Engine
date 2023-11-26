#pragma once
#include <string>
#include <vector>

class Shader
{
public:
	static std::vector<char> ReadFile(const std::string& filename);
};
