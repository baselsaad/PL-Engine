#include "pch.h"
#include "VulkanAPI.h"


#include "GraphhicsPipeline.h"
#include "RenderPass.h"
#include "Shader.h"
#include "SwapChain.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"


namespace PAL
{
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//													VulkanMemoryAllocator							  																	      //
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	VmaAllocator VulkanMemoryAllocator::s_Allocator;
	uint64_t VulkanMemoryAllocator::s_TotalAllocatedBytes;

	VulkanMemoryAllocator::VulkanMemoryAllocator(const std::string& name)
		: m_Name(name)
	{
	}

	//@TODO: Assert all parameters

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


	void VulkanMemoryAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_Allocator, allocation);
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//													VulkanUtilities							  																	      //
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


	uint32_t VulkanUtilities::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		auto phyDevice = VulkanContext::GetVulkanDevice()->GetPhysicalDevice()->GetVkPhysicalDevice();

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(phyDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ASSERT(false, "failed to find suitable memory type!");
	}

	void VulkanUtilities::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const SharedPtr<CommandBuffer>& cmBuffer)
	{
		auto vulkanDevice = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = cmBuffer->GetCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(vulkanDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(VulkanContext::GetVulkanDevice()->GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VulkanContext::GetVulkanDevice()->GetVkGraphicsQueue());

		vkFreeCommandBuffers(vulkanDevice, cmBuffer->GetCommandPool(), 1, &commandBuffer);
	}





}