#include "pch.h"
#include "VulkanVertexBuffer.h"

#include "GraphhicsPipeline.h"
#include "RenderPass.h"
#include "Shader.h"
#include "SwapChain.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "CommandBuffer.h"
#include "VulkanMemoryAllocator.h"
#include "VulkanAPI.h"

#include "Renderer/RuntimeRenderer.h"

namespace PAL
{

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
		: m_Size(size)
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VulkanMemoryAllocator allocator("VertexBuffer");

		VkBufferCreateInfo vertexBufferCreateInfo = {};
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.size = size;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		m_VmaAllocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_VertexBuffer);
	}

	void VulkanVertexBuffer::DestroyBuffer()
	{
		VulkanMemoryAllocator allocator("VertexBuffer");
		allocator.DestroyBuffer(m_VertexBuffer, m_VmaAllocation);
	}

	void VulkanVertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
	{
		VulkanMemoryAllocator allocator("VulkanVertexBuffer");
		uint8_t* pData = allocator.MapMemory<uint8_t>(m_VmaAllocation);
		memcpy(pData, (uint8_t*)data + offset, size);
		allocator.UnmapMemory(m_VmaAllocation);
	}
}