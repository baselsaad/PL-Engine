#pragma once
#include "VulkanAPI.h"
#include "Renderer/IndexBuffer.h"

namespace PAL
{
	class CommandBuffer;

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(void* data, uint32_t size);
		~VulkanIndexBuffer();

		virtual void DestroyBuffer() override;

		inline const VkBuffer GetVkIndexBuffer() { return m_IndexBuffer; }
		inline uint32_t GetSize() { return m_Size; }

	private:
		VmaAllocation m_VmaAllocation;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		uint32_t m_Size;
	};

}
