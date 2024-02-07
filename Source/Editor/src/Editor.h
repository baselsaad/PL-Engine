#pragma once
#include "imgui.h"
#include "Platform/PlatformEntry.h"
#include "UI/HierarchyPanel.h"

namespace PAL
{
	struct VulkanImage;
	class Image2D;
	class VulkanSwapChain;

	enum class ImGuiFonts
	{
		Regular,
		Bold,
		Light,
		BoldItalic,
		RegularItalic,
		LightItalic,
	};

	class Editor : public EngineApplication
	{
	public:
		Editor();
		~Editor();

		virtual void Init() override;
		virtual void SetupInput(EventHandler& eventHandler) override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnShutdown() override;

		void SetupVulkan();

		void InitImGui();

		void SetStyleLight();
		void SetStyleDark();
		    
		void BeginFrame();

		void RenderImGui();
		void RenderMenuBar();
		void RenderStatsPanel();
		void RenderSettingsPanel();
		void RenderViewport();

		void EndFrame();

		glm::vec2 GetMousePosRelativeViewport(const ImVec2& imguiMousePos, const ImVec2& viewportPos);
		void OnMousePressed(const MouseButtonPressedEvent& mouseEvent);

		inline ImFont* GetFont(ImGuiFonts font = ImGuiFonts::Regular) { return m_ImGuiFonts[font]; }


	private:
		HierarchyPanel m_HierarchyPanel;
		std::map<ImGuiFonts, ImFont*> m_ImGuiFonts;

		// Move Later
		VkRenderPass m_ImGuiRenderPass = VK_NULL_HANDLE;
		VkDescriptorSet m_MainViewportTexID = VK_NULL_HANDLE;
		SharedPtr<VulkanSwapChain> m_Swapchain;
		
		ImVec2 m_ViewportPosition;
		std::array< ImVec2, 2> m_ViewportBound;
		glm::vec2 m_MousePos = { 0.0f, 0.0f };
	};
}



