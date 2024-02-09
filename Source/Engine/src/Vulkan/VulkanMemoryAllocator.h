#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace PAL
{
	class VulkanDevice;

	class VulkanMemoryAllocator
	{

	public:
		VulkanMemoryAllocator(const char* name);

		static void Init(const SharedPtr<VulkanDevice>& device);
		static void Shutdown();

		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer);
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

		VmaAllocation AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage, VkMemoryPropertyFlags propertyFlags = 0);
		void DestroyImage(VkImage image, VmaAllocation allocation);


		template<typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* mappedMemory;
			vmaMapMemory(s_Allocator, allocation, (void**)&mappedMemory);
			return mappedMemory;
		}

		void UnmapMemory(VmaAllocation allocation);

	private:
		static VmaAllocator s_Allocator;
		static uint64_t s_TotalAllocatedBytes;

		const char* m_Name;
	};

}