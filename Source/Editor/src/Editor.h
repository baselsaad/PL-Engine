#pragma once
#include "imgui.h"

namespace PAL
{
	struct VulkanImage;
	class Image2D;

	class Editor
	{
	public:
		static Editor& GetInstance();

		Editor();
		~Editor();
		void Shutdown();

		void CreateRenderPass();

		void OnRenderImGui(VulkanImage* image);
		
		void BeginFrame();
		void EndFrame();

		inline ImVec2 GetViewportSize() const { return m_ViewportSize; }

	private:
		VkRenderPass m_ImGuiRenderPass;
		// @TODO : Move this to a SceneRenderer class
		ImVec2 m_ViewportSize;
	};
}