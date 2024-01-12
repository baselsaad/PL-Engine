#pragma once
#include "imgui.h"

namespace PAL
{
	struct VulkanImage;
	class Image2D;

	enum class ImGuiFonts
	{
		Regular,
		Bold,
		Light,
		BoldItalic,
		RegularItalic,
		LightItalic,
	};

	class Editor
	{
	public:
		static Editor& GetInstance();

		Editor();
		~Editor();
		void Shutdown();

		void CreateRenderPass();

		void SetDarkThemeColors();
		void OnRenderImGui(VulkanImage* image);
		
		void BeginFrame();
		void EndFrame();

		inline const ImVec2& GetViewportSize() const { return m_ViewportSize; }
		inline ImFont* GetFont(ImGuiFonts font = ImGuiFonts::Regular) { return m_ImGuiFonts[font]; }
	private:
		VkRenderPass m_ImGuiRenderPass;
		// @TODO : Move this to a SceneRenderer class
		ImVec2 m_ViewportSize;

		std::map<ImGuiFonts, ImFont*> m_ImGuiFonts;
	};
}