#include "pch.h"
#include "IndexBuffer.h"

#include "VertexBuffer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanRenderer.h"
#include "CommandBuffer.h"
#include "VulkanAPI.h"


namespace PAL
{

	VulkanIndexBuffer::VulkanIndexBuffer(const SharedPtr<CommandBuffer>& cmBuffer, void* data, uint32_t size)
		: m_Size(size)
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();
		VulkanMemoryAllocator allocator("IndexBuffer");

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

		uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
		memcpy(destData, data, size);
		allocator.UnmapMemory(stagingBufferAllocation);

		VkBufferCreateInfo indexBufferCreateInfo = {};
		indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferCreateInfo.size = size;
		indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		m_VmaAllocation = allocator.AllocateBuffer(indexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_IndexBuffer);

		VulkanUtilities::CopyBuffer(stagingBuffer, m_IndexBuffer, size, cmBuffer);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		//DestroyBuffer();
	}

	void VulkanIndexBuffer::DestroyBuffer()
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VulkanMemoryAllocator allocator("IndexBuffer");
		allocator.DestroyBuffer(m_IndexBuffer, m_VmaAllocation);
	}

}