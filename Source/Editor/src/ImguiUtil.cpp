#include "pch.h"
#include "ImguiUtil.h"


namespace ImGuiUtil
{
	glm::vec2 ImGuiUtil::ConvertImGuiVec2ToGlm(const ImVec2& vec)
	{
		return glm::vec2(vec.x, vec.y);
	}

	ImVec2 ConvertGlmToImgui(const glm::vec2& vec)
	{
		return ImVec2(vec.x, vec.y);
	}
}


