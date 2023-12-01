#pragma once
#include "VulkanAPI.h"

namespace PL_Engine
{
	class CommandBuffer;

	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec3 Color;

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, Pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, Color);

			return attributeDescriptions;
		}
	};

	class VulkanVertexBuffer
	{

	public:
		VulkanVertexBuffer(const SharedPtr<CommandBuffer>& commandBuffer);
		VulkanVertexBuffer(const SharedPtr<CommandBuffer>& commandBuffer, void* data, uint32_t size);
		void DestroyBuffer();

		inline VkBuffer GetVkVertexBuffer() { return m_VertexBuffer; }
		inline const VkBuffer GetVkVertexBuffer() const { return m_VertexBuffer; }
		inline const std::vector<Vertex>& GetVertcies() const { return m_Vertices; }

	private:
		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		VkBuffer m_StagingBuffer;
		VkDeviceMemory m_StagingBufferMemory;

		const std::vector<Vertex> m_Vertices =
		{
			{ {-0.5f ,-0.5f ,0.0f},  {1.0f, 0.0f, 0.0f} },
			{ {0.5f  ,-0.5f ,0.0f},  {0.0f, 1.0f, 0.0f} },
			{ {0.5f  ,0.5f  ,0.0f},  {0.0f, 0.0f, 1.0f} },
			{ {-0.5f ,0.5f  ,0.0f},  {1.0f, 1.0f, 1.0f} },
		};
	};

}