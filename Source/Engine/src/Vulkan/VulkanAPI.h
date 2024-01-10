#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

/*
	#include "GraphhicsPipeline.h"
	#include "RenderPass.h"
	#include "Shader.h"
	#include "SwapChain.h"
	#include "VulkanContext.h"
	#include "VulkanDevice.h"
	#include "CommandBuffer.h"
	#include "VertexBuffer.h"
*/


namespace PAL
{
	class CommandBuffer;
	class VulkanDevice;

	class VulkanUtilities
	{
	public:
		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const SharedPtr<CommandBuffer>& cmBuffer);
	};


	class VulkanMemoryAllocator
	{

	public:
		VulkanMemoryAllocator(const std::string& name);

		static void Init(const SharedPtr<VulkanDevice>& device);
		static void Shutdown();

		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer);
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

		void DestroyImage(VkImage image, VmaAllocation allocation);
		
		
		template<typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* mappedMemory;
			vmaMapMemory(s_Allocator, allocation, (void**)&mappedMemory);
			return mappedMemory;
		}

		void UnmapMemory(VmaAllocation allocation);

		VmaAllocation AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage);
	private:
		static VmaAllocator s_Allocator;
		static uint64_t s_TotalAllocatedBytes;

		std::string m_Name;
	};
}