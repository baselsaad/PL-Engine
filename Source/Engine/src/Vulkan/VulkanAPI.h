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
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanFrameBuffer.h"
#include "VulkanMemoryAllocator.h"
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
		static void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	};
	
}