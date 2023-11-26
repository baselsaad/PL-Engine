#pragma once
#include "Vulkan.h"

namespace PL_Engine
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice();

		QueueFamilyIndices FindQueueFamilies();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport();

		inline VkPhysicalDevice GetVkPhysicalDevice() { return m_PhysicalDevice; }
		inline const std::vector<const char*>& GetVulkanDeviceExtensions() const { return m_DeviceExtensions; }

	private:
		VkPhysicalDevice m_PhysicalDevice;

		const std::vector<const char*> m_DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(const SharedPtr<VulkanPhysicalDevice>& vulkanDevice);

		inline VkDevice GetVkDevice() { return m_Device; }
		inline const SharedPtr<VulkanPhysicalDevice>& GetPhysicalDevice() { return m_PhysicalDevice; }

		inline VkQueue GetVkGraphicsQueue() { return m_GraphicsQueue; }
		inline VkQueue GetVkPresentQueue() { return m_PresentQueue; }

	private:
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		SharedPtr<VulkanPhysicalDevice> m_PhysicalDevice;
	};
}



