#pragma once
#include "Vulkan/VulkanRenderer.h"

namespace PL_Engine
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class CommandBuffer;
	struct QuadVertex;

	class BatchRenderer
	{
	public:
		BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer);
		~BatchRenderer();

		void StartNewBatch();
		void BatchNewQuadVertices(const glm::mat4& transform, const glm::vec3& color);
		void BindCurrentBatch();

		inline bool ShouldDrawBatch() { return m_QuadIndexCount >= s_MaxIndices; }
		inline const SharedPtr<VulkanVertexBuffer>& GetVertexBuffer() { return m_QuadVertexBuffer[VulkanAPI::GetCurrentFrame()]; }
		inline const SharedPtr<VulkanIndexBuffer>& GetIndexBuffer() { return m_QuadIndexBuffer; }
		inline uint32_t GetIndexCount() { return m_QuadIndexCount; }

	private:
		static const uint32_t s_MaxQuads = 10000;
		static const uint32_t s_MaxVertices = s_MaxQuads * 4;
		static const uint32_t s_MaxIndices = s_MaxQuads * 6;

		std::vector<SharedPtr<VulkanVertexBuffer>> m_QuadVertexBuffer;
		SharedPtr<VulkanIndexBuffer> m_QuadIndexBuffer;

		uint32_t m_QuadIndexCount = 0;
		std::vector<QuadVertex*> m_QuadVertexBufferBase;
		QuadVertex* m_QuadVertexBufferPtr = nullptr;

		glm::vec4 m_QuadVertexPositions[4];
	};

}