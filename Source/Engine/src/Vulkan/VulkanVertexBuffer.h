#pragma once
#include "VulkanAPI.h"
#include "Renderer/VertexBuffer.h"

namespace PAL
{
	class CommandBuffer;

	struct VulkanQuadVertex
	{
		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(QuadVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			// Position attribute
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(QuadVertex, Pos);

			// Color attribute
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(QuadVertex, Color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(QuadVertex, ObjectId);

			return attributeDescriptions;
		}
	};

	class VulkanVertexBuffer : public VertexBuffer
	{

	public:
		VulkanVertexBuffer(uint32_t size);
		virtual void DestroyBuffer() override;
		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) override;

		inline VkBuffer GetVkVertexBuffer() { return m_VertexBuffer; }
		inline const VkBuffer GetVkVertexBuffer() const { return m_VertexBuffer; }

	private:
		uint32_t m_Size = 0;
		VkBuffer m_VertexBuffer;
		VmaAllocation m_VmaAllocation;
	};

}