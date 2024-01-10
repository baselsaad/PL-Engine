#pragma once
#include "imgui.h"

namespace PAL
{
	class Editor
	{
	public:
		static Editor& GetInstance();

		Editor();
		~Editor();
		void Shutdown();

		void OnRenderImGui();

		void BeginFrame();
		void EndFrame();
	};
}