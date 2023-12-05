#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{
	class CommandBuffer;

	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(const SharedPtr<CommandBuffer>& cmBuffer, void* data, uint32_t size);
		~VulkanIndexBuffer();

		void DestroyBuffer();

		inline const VkBuffer GetVkIndexBuffer() { return m_IndexBuffer; }
		inline uint32_t GetSize() { return m_Size; }

	private:
		VmaAllocation m_VmaAllocation;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		uint32_t m_Size;
	};

}
