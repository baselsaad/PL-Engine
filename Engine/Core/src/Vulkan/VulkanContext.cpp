#include "pch.h"
#include "VulkanContext.h"


#include <GLFW/glfw3.h>
#include "Core/Engine.h"
#include "VulkanDevice.h"
#include "SwapChain.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Static
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	///std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	switch (messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		Debug::Log("validation layer: {0}", pCallbackData->pMessage);
		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		Debug::Info("validation layer: {0}", pCallbackData->pMessage);
		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Debug::LogWarn("validation layer: {0}\n", pCallbackData->pMessage);
		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		Debug::LogError("validation layer: {0}\n", pCallbackData->pMessage);
		break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
		Debug::Critical("validation layer: {0}\n", pCallbackData->pMessage);
		break;
		default:
		break;
	}

	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
//  VulkanRenderer
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace PL_Engine
{
	VkInstance VulkanContext::s_VulkanInstance = nullptr;
	VkDebugUtilsMessengerEXT VulkanContext::s_DebugMessenger;
	VkSurfaceKHR VulkanContext::s_Surface;

	SharedPtr<VulkanDevice> VulkanContext::s_VulkanDevice;
	SharedPtr<VulkanPhysicalDevice> VulkanContext::s_VulkanPhysicalDevice;
	SharedPtr<VulkanSwapChain> VulkanContext::s_SwapChain;

	static const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	void VulkanContext::Init()
	{
		ASSERT(CheckValidationLayerSupport(), "validation layers requested, but not available!");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "PL-Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

#if DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
#endif

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &s_VulkanInstance));
		SetupDebugMessenger();

		CreateSurface();

		s_VulkanPhysicalDevice = MakeShared<VulkanPhysicalDevice>();
		s_VulkanDevice = MakeShared<VulkanDevice>(s_VulkanPhysicalDevice);

	}

	void VulkanContext::Shutdown()
	{
#if DEBUG
		DestroyDebugUtilsMessengerEXT(s_VulkanInstance, s_DebugMessenger, nullptr);
#endif
		s_SwapChain->CleanupSwapChain();
		vkDestroyDevice(s_VulkanDevice->GetVkDevice(), nullptr);
		vkDestroySurfaceKHR(s_VulkanInstance, s_Surface, nullptr);
		vkDestroyInstance(s_VulkanInstance, nullptr);

		
	}

	void VulkanContext::SetupDebugMessenger()
	{
#ifndef DEBUG
		return;
#endif 

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(s_VulkanInstance, &createInfo, nullptr, &s_DebugMessenger));
	}

	void VulkanContext::CreateSurface()
	{
		VK_CHECK_RESULT(glfwCreateWindowSurface(s_VulkanInstance, Engine::Get()->GetWindow()->GetWindowHandle(), nullptr, &s_Surface));
	}

	void VulkanContext::CreateVulkanSwapChain()
	{
		s_SwapChain = MakeShared<VulkanSwapChain>(s_VulkanDevice);
		s_SwapChain->Create();
		s_SwapChain->CreateImageViews();
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	std::vector<const char*> VulkanContext::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

}
