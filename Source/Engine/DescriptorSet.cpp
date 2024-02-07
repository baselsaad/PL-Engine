#include "pch.h"
#include "DescriptorSet.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanRenderer.h"

namespace PAL
{
	DescriptorSet::DescriptorSet()
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		// @TODO custom layout
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0; // should match the binding in your shader
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Shader stage to bind to

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;

		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout);

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // Adjust based on your resource type
		poolSize.descriptorCount = 1; // Specify the number of descriptors

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = VulkanAPI::GetMaxFramesInFlight(); // descriptor sets for each frame

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));

		m_DescriptorSets.resize(VulkanAPI::GetMaxFramesInFlight());

		for (size_t i = 0; i < m_DescriptorSets.size(); ++i)
		{
			CreateStorageBuffer(m_StorageBuffers[i]);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_DescriptorSetLayout;

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSets[i]));

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_StorageBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			// @TODO custom based on resource type
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = 0; // Should match the binding in shader
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void DescriptorSet::Shutdown()
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		// Destroy the descriptor sets
		vkFreeDescriptorSets(device, m_DescriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());

		// Destroy the descriptor set layout and pool
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}

	void DescriptorSet::CreateStorageBuffer(VkBuffer& buffer)
	{
		auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//bufferInfo.size = sizeof(/* Your buffer data type */);
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory));
		//VK_CHECK_RESULT(vkBindBufferMemory(device, *buffer, m_Memory, 0));
	}

	void DescriptorSet::Bind(VkPipelineLayout pipelineLayout, size_t frameIndex)
	{
		vkCmdBindDescriptorSets(VulkanContext::GetVulkanDevice()->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_DescriptorSets[frameIndex], 0, nullptr);
	}
}
