#pragma once
#include <vulkan/vulkan.h>

namespace PL_Engine
{

	class VulkanContext
	{
	public:
		static void Init();
		static void CreateSurface();
		static void Shutdown();
		static std::vector<const char*> GetRequiredExtensions();

		inline static VkInstance GetVulkanInstance() { return s_VulkanInstance; }
		inline static VkSurfaceKHR GetSurface() { return s_Surface; }

	private:
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static bool CheckValidationLayerSupport();
		static void SetupDebugMessenger();


	private:
		static VkInstance s_VulkanInstance;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static VkSurfaceKHR s_Surface;

	};

}