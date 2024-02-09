#include "pch.h"
#include "VulkanMemoryAllocator.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanVertexBuffer.h"

namespace PAL
{
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//													VulkanMemoryAllocator							  																	      //
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	VmaAllocator VulkanMemoryAllocator::s_Allocator;
	uint64_t VulkanMemoryAllocator::s_TotalAllocatedBytes;

	VulkanMemoryAllocator::VulkanMemoryAllocator(const char* name)
		: m_Name(name)
	{
	}

	void VulkanMemoryAllocator::Init(const SharedPtr<VulkanDevice>& device)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = device->GetPhysicalDevice()->GetVkPhysicalDevice();
		allocatorInfo.device = device->GetVkDevice();
		allocatorInfo.instance = VulkanContext::GetVulkanInstance();

		vmaCreateAllocator(&allocatorInfo, &s_Allocator);
	}

	void VulkanMemoryAllocator::Shutdown()
	{
		vmaDestroyAllocator(s_Allocator);
	}

	VmaAllocation VulkanMemoryAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer)
	{
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		VK_CHECK_RESULT(vmaCreateBuffer(s_Allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr));

		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_Allocator, allocation, &allocInfo);
		s_TotalAllocatedBytes += allocInfo.size;

		return allocation;
	}

	void VulkanMemoryAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
		vmaDestroyBuffer(s_Allocator, buffer, allocation);
	}

	void VulkanMemoryAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		vmaDestroyImage(s_Allocator, image, allocation);
	}

	void VulkanMemoryAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_Allocator, allocation);
	}

	VmaAllocation VulkanMemoryAllocator::AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage, VkMemoryPropertyFlags propertyFlags)
	{
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = usage;
		//allocCreateInfo.requiredFlags = propertyFlags;

		VmaAllocation allocation;
		vmaCreateImage(s_Allocator, &imageCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_Allocator, allocation, &allocInfo);

		return allocation;
	}

}