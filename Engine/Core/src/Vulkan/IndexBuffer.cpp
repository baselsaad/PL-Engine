#include "pch.h"
#include "IndexBuffer.h"

#include "VertexBuffer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"


namespace PL_Engine
{

	VulkanIndexBuffer::VulkanIndexBuffer(const SharedPtr<CommandBuffer>& commandBuffer, uint32_t count)
		: m_Count(count)
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_Indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);
		VulkanUtilities::CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize, commandBuffer);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const SharedPtr<CommandBuffer>& commandBuffer, void* data, uint32_t count)
		: m_Count(count)
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VkDeviceSize bufferSize = count * sizeof(uint16_t);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* mappedData;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &mappedData);
		memcpy(mappedData, data, (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);
		VulkanUtilities::CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize, commandBuffer);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanIndexBuffer::DestroyBuffer()
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		vkDestroyBuffer(device, m_IndexBuffer, nullptr);
		vkFreeMemory(device, m_IndexBufferMemory, nullptr);
	}

}