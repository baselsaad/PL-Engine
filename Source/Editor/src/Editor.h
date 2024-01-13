#pragma once
#include "imgui.h"
#include "Platform/PlatformEntry.h"

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

		void Init() override;
		void OnUpdate(float deltaTime) override;
		void OnShutdown() override;
		
		void SetupVulkan();

		void InitImGui();

		void SetStyleLight();
		void SetStyleDark();
		
		void BeginFrame();
		void OnRenderImGui(VulkanImage* image);
		void EndFrame();

		inline ImFont* GetFont(ImGuiFonts font = ImGuiFonts::Regular) { return m_ImGuiFonts[font]; }
	
	private:
		std::map<ImGuiFonts, ImFont*> m_ImGuiFonts;

		// Move Later
		VkRenderPass m_ImGuiRenderPass = VK_NULL_HANDLE;
		VkDescriptorSet m_MainViewportTexID = VK_NULL_HANDLE;
		SharedPtr<VulkanSwapChain> m_Swapchain; // use weakptr
	};
}



