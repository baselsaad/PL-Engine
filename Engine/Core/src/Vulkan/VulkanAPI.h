#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

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


namespace PL_Engine
{
	class CommandBuffer;

	class VulkanUtilities
	{
	public:
		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const SharedPtr<CommandBuffer>& cmBuffer);
		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
}