#pragma once

namespace PAL
{
	class VulkanDevice;
	class VulkanPhysicalDevice;
	class VulkanSwapChain;

	class VulkanContext
	{
	public:
		static void Init(void* windowHandle);
		static void CreateSurface(void* windowHandle);
		static void Shutdown();

		inline static VkInstance GetVulkanInstance() { return s_VulkanInstance; }
		inline static VkSurfaceKHR GetSurface() { return s_Surface; }

		inline static const SharedPtr<VulkanDevice>& GetVulkanDevice() { return s_VulkanDevice; }

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
	};

}