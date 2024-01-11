#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "SwapChain.h"

namespace PAL
{
	//-------------------------------------------------------------------------------------------------------------
	// VulkanPhysicalDevice
	//-------------------------------------------------------------------------------------------------------------
	VulkanPhysicalDevice::VulkanPhysicalDevice()
		: m_PhysicalDevice(VK_NULL_HANDLE)
	{
		auto instance = VulkanContext::GetVulkanInstance();

		uint32_t deviceCount = 0;

		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		ASSERT(deviceCount > 0, "failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			m_PhysicalDevice = device;
			if (IsDeviceSuitable(device))
			{
				break;
			}
			else
			{
				m_PhysicalDevice = VK_NULL_HANDLE;
			}
		}

		ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!");
		
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
		m_DeviceName = properties.deviceName;
		m_Vendor = properties.vendorID;
		m_DriverVersion = properties.driverVersion;
	}

	QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilies()
	{
		QueueFamilyIndices indices;

		// Logic to find queue family indices to populate struct with
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, VulkanContext::GetSurface(), &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break; // No need to continue if both families are found
			}
		}

		return indices;
	}

	bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		/*
		*	// Get Device Information
		*	VkPhysicalDeviceProperties deviceProperties;
		*	VkPhysicalDeviceFeatures deviceFeatures;
		*	vkGetPhysicalDeviceProperties(device, &deviceProperties);
		*	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		*
		*	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		*		deviceFeatures.geometryShader;
		*
		*/

		QueueFamilyIndices indices = FindQueueFamilies();
		bool extensionsSupported = CheckDeviceExtensionSupport();

		// SwapChain Support
		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = VulkanSwapChain::QuerySwapChainSupport(device, VulkanContext::GetSurface());
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	

	bool VulkanPhysicalDevice::CheckDeviceExtensionSupport()
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}



	//-------------------------------------------------------------------------------------------------------------


	//-------------------------------------------------------------------------------------------------------------
	// VulkanDevice
	//-------------------------------------------------------------------------------------------------------------

	VulkanDevice::VulkanDevice(const SharedPtr<VulkanPhysicalDevice>& vulkanDevice)
		: m_Device(nullptr),
		m_PhysicalDevice(vulkanDevice)
	{
		QueueFamilyIndices indices = m_PhysicalDevice->FindQueueFamilies();

		/*VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;*/

		// Creating the presentation queue
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//Specifying used device features
		VkPhysicalDeviceFeatures deviceFeatures{};

		//Creating the logical device
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.enabledLayerCount = 0;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		
		//Enabling device extensions
		auto extensions = m_PhysicalDevice->GetVulkanDeviceExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice->GetVkPhysicalDevice(), &createInfo, nullptr, &m_Device));

		// Retrieving queue handles
		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
	}

	void VulkanDevice::Shutdown()
	{
		m_CommandBuffer->Shutdown();

		vkDestroyDevice(m_Device, nullptr);
	}

	void VulkanDevice::CreateMainCommandBuffer()
	{
		m_CommandBuffer = NewShared<CommandBuffer>();
	}

	//-------------------------------------------------------------------------------------------------------------
}


