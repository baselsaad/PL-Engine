#pragma once
#include "VulkanAPI.h"
#include "VulkanDevice.h"


namespace PAL
{
	class Window;
	class RenderPass;
	class VulkanFramebuffer;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanSwapChain
	{
	public:
		using CallbackType = std::function<void(uint32_t newWidth, uint32_t newHeight)>;

		VulkanSwapChain() = default;
		~VulkanSwapChain();
		VulkanSwapChain(const SharedPtr<VulkanDevice>& device);

		void Create();
		void RecreateSwapChain();
		void PresentFrame();
		void CleanupSwapChain();


		uint32_t AcquireNextImage(const SharedPtr<RenderPass>& renderpass);
		static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		inline const VkSwapchainKHR GetVkSwapChain() const { return m_SwapChain; }
		inline const std::vector<VkImage>& GetSwapChainImages() const { return m_SwapChainImages; }
		inline const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_SwapChainImageViews; }
		inline const VkExtent2D& GetSwapChainExtent() const { return m_SwapChainExtent; }
		inline const VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
		inline const std::vector<VkFence>& GetInFlightFence() { return m_InFlightFence; }
		inline const uint32_t& GetImageIndex() { return m_ImageIndex; }

		inline uint32_t GetCurrentFrame() { return m_CurrentFrame; }

		inline void BindResizeCallback(CallbackType&& callback) { m_ResizeCallbacks.emplace_back(callback); }
		// @TODO : UnBindResizeCallback
	private:
		void CreateSyncObjects();
		void CreateImageViews();

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		SharedPtr<VulkanDevice> m_Device;

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		std::vector<VkImageView> m_SwapChainImageViews;

		// Semaphores
		std::vector<VkSemaphore> m_ImageAvailableSemaphore;
		std::vector<VkSemaphore> m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFence;

		uint32_t m_ImageIndex;
		uint32_t m_CurrentFrame;

		std::vector<CallbackType> m_ResizeCallbacks;
	};
}
