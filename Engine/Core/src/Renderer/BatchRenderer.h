#pragma once
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/VertexBuffer.h"

namespace PL_Engine
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class CommandBuffer;
	struct QuadVertex;

	struct QuadBatch
	{
		static const uint32_t s_MaxQuads = 10000;
		static const uint32_t s_MaxVertices = s_MaxQuads * 4;

		int frameID = 0;//for debug
		SharedPtr<VulkanVertexBuffer> VertexBuffer;
		bool IsFull = false;

		QuadBatch(int frameIndex)
			: frameID(frameIndex)
		{
			VertexBuffer = MakeShared<VulkanVertexBuffer>(s_MaxVertices * sizeof(QuadVertex));
		}
	};

	class BatchRenderer
	{
	public:
		BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer);
		~BatchRenderer();

		void Begin();
		void End();

		void FindOrCreateNewBatch();
		void BatchNewQuadVertices(const glm::mat4& transform, const glm::vec3& color);
		void BindCurrentBatch();

		inline bool IsBatchFull() { return m_QuadIndexCount >= s_MaxIndices; }
		const SharedPtr<VulkanVertexBuffer>& GetVertexBuffer(); 
		inline const SharedPtr<VulkanIndexBuffer>& GetIndexBuffer() { return m_QuadIndexBuffer; }
		inline uint32_t GetIndexCount() { return m_QuadIndexCount; }

	private:
		static const uint32_t s_MaxIndices = QuadBatch::s_MaxQuads * 6;

		std::vector<QuadVertex*> m_QuadVertexBufferBase;
		QuadVertex* m_QuadVertexBufferPtr = nullptr;

		SharedPtr<VulkanIndexBuffer> m_QuadIndexBuffer;

		std::vector<QuadBatch> m_Batches[VulkanAPI::GetMaxFramesInFlight()]; 
		int m_CurrentQuadBatch[VulkanAPI::GetMaxFramesInFlight()];

		uint32_t m_QuadIndexCount = 0;
		glm::vec4 m_QuadVertexPositions[4];

		friend struct QuadBatch;
	};

}

