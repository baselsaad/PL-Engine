#pragma once
#include "VulkanAPI.h"
#include "VulkanDevice.h"


namespace PL_Engine
{
	class Window;
	class RenderPass;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain() = default;
		~VulkanSwapChain();
		VulkanSwapChain(const std::shared_ptr<VulkanDevice>& device);

		void Create();
		void CreateImageViews();
		void CreateFramebuffers(VkRenderPass renderPass);
		void CleanupSwapChain();
		void RecreateSwapChain(const SharedPtr<RenderPass>& renderPass);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		inline const VkSwapchainKHR GetVkSwapChain() const { return m_SwapChain; }
		inline const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		inline const std::vector<VkFramebuffer>& GetSwapChainFramebuffers() const { return m_SwapChainFramebuffers; }
		inline const VkExtent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		inline const VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }

	private:
		std::shared_ptr<VulkanDevice> m_Device;

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	};
}
