#include "pch.h"
#include "VertexBuffer.h"

#include "GraphhicsPipeline.h"
#include "RenderPass.h"
#include "Shader.h"
#include "SwapChain.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "CommandBuffer.h"
#include "vulkan/vulkan_core.h"

namespace PL_Engine
{

	VulkanVertexBuffer::VulkanVertexBuffer(const SharedPtr<CommandBuffer>& commandBuffer)
	{
		auto vulkanDevice = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);

		void* data;
		vkMapMemory(vulkanDevice, m_StagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_Vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(vulkanDevice, m_StagingBufferMemory);

		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);
		VulkanUtilities::CopyBuffer(m_StagingBuffer, m_VertexBuffer, bufferSize, commandBuffer);

		vkDestroyBuffer(vulkanDevice, m_StagingBuffer, nullptr);
		vkFreeMemory(vulkanDevice, m_StagingBufferMemory, nullptr);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const SharedPtr<CommandBuffer>& commandBuffer, void* data, uint32_t size)
	{
		auto vulkanDevice = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VkDeviceSize bufferSize = size;

		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);

		void* mappedData;
		vkMapMemory(vulkanDevice, m_StagingBufferMemory, 0, bufferSize, 0, &mappedData);
		memcpy(mappedData, data, (size_t)bufferSize);
		vkUnmapMemory(vulkanDevice, m_StagingBufferMemory);

		VulkanUtilities::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);
		VulkanUtilities::CopyBuffer(m_StagingBuffer, m_VertexBuffer, bufferSize, commandBuffer);

		vkDestroyBuffer(vulkanDevice, m_StagingBuffer, nullptr);
		vkFreeMemory(vulkanDevice, m_StagingBufferMemory, nullptr);
	}

	void VulkanVertexBuffer::DestroyBuffer()
	{
		auto vulkanDevice = VulkanContext::GetVulkanDevice()->GetVkDevice();

		vkDestroyBuffer(vulkanDevice, m_VertexBuffer, nullptr);
		vkFreeMemory(vulkanDevice, m_VertexBufferMemory, nullptr);
	}

}