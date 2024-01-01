#pragma once
#include "VulkanAPI.h"

namespace PAL
{
	class VulkanDevice;
	class VulkanPhysicalDevice;
	class VulkanSwapChain;

	class VulkanContext
	{
	public:
		static void Init();
		static void CreateSurface();
		static void CreateVulkanSwapChain();
		static void Shutdown();

		inline static VkInstance GetVulkanInstance() { return s_VulkanInstance; }
		inline static VkSurfaceKHR GetSurface() { return s_Surface; }

		inline static const SharedPtr<VulkanDevice>& GetVulkanDevice() { return s_VulkanDevice; }
		inline static SharedPtr<VulkanSwapChain>& GetSwapChain() { return s_SwapChain; };

	private:
		static std::vector<const char*> GetRequiredExtensions();
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static bool CheckValidationLayerSupport();
		static void SetupDebugMessenger();


	private:
		static VkInstance s_VulkanInstance;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static VkSurfaceKHR s_Surface;

		static SharedPtr<VulkanDevice> s_VulkanDevice;
		static SharedPtr<VulkanPhysicalDevice> s_VulkanPhysicalDevice;
		static SharedPtr<VulkanSwapChain> s_SwapChain;
	};

}