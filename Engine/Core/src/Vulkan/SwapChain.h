#pragma once
#include "VulkanAPI.h"
#include "VulkanDevice.h"


namespace PAL
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
		VulkanSwapChain(const SharedPtr<VulkanDevice>& device);

		void Create();
		void CreateSyncObjects();

		void CreateImageViews();
		void CreateFramebuffers(VkRenderPass renderPass);
		void CleanupSwapChain();
		void RecreateSwapChain(const SharedPtr<RenderPass>& renderPass);
		void PresentFrame(const SharedPtr<RenderPass>& renderpass, const SharedPtr<CommandBuffer>& commandBuffer);
		
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		uint32_t AcquireNextImage(const SharedPtr<RenderPass>& renderpass);

		static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		inline const VkSwapchainKHR GetVkSwapChain() const { return m_SwapChain; }
		inline const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		inline const std::vector<VkFramebuffer>& GetSwapChainFramebuffers() const { return m_SwapChainFramebuffers; }
		inline const VkExtent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		inline const VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		inline const std::vector<VkFence>& GetInFlightFence() { return m_InFlightFence; }
		inline const uint32_t& GetImageIndex() { return m_ImageIndex; }
	private:
		SharedPtr<VulkanDevice> m_Device;

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		// Semaphores
		std::vector<VkSemaphore> m_ImageAvailableSemaphore;
		std::vector<VkSemaphore> m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFence;

		uint32_t m_ImageIndex;

	};
}
