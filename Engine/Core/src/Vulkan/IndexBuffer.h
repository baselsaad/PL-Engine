#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{

	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(const SharedPtr<CommandBuffer>& commandBuffer, uint32_t count = 6);
		VulkanIndexBuffer(const SharedPtr<CommandBuffer>& commandBuffer, void* data, uint32_t count);

		void DestroyBuffer();

		inline const std::vector<uint16_t>& GetIndices() { return m_Indices; }
		inline const VkBuffer GetVkIndexBuffer() { return m_IndexBuffer; }
		inline uint32_t GetCount() { return m_Count; }

	private:
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		uint32_t m_Count;

		const std::vector<uint16_t> m_Indices = { 0, 1, 2, 2, 3, 0 };
	};

}
