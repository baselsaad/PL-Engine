#pragma once
#include "imgui.h"

namespace ImGuiUtil
{
	glm::vec2 ConvertImGuiVec2ToGlm(const ImVec2& vec);
	ImVec2 ConvertGlmToImgui(const glm::vec2& vec);

	inline glm::vec2 FastGlmConversion(ImVec2* vec)
	{
		// since they have same memory layout 
		return *(glm::vec2*)(vec);
	}

	inline ImVec2 FastVecToImVec(glm::vec2* vec)
	{
		return *(ImVec2*)(vec);
	}
}