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
		SharedPtr<VulkanVertexBuffer> VertexBuffer;
		int FrameIndex = 0; // only for debug

		QuadBatch(int frameIndex = 0);
	};

	class BatchRenderer
	{
	public:
		BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer);
		~BatchRenderer();

		void Begin();
		void End();
		void AddQuadToBatch(const glm::mat4& transform, const glm::vec3& color);
		void BindCurrentQuadBatch();
		void FindOrCreateNewQuadBatch();

		const SharedPtr<VulkanVertexBuffer>& GetVertexBuffer(); 

		inline bool ShouldDrawCurrentBatch() { return m_QuadBatchingData.IndexCount >= m_QuadBatchingData.s_MaxIndices; };
		inline const SharedPtr<VulkanIndexBuffer>& GetIndexBuffer() { return m_QuadBatchingData.IndexBuffer; }
		inline uint32_t GetIndexCount() { return m_QuadBatchingData.IndexCount; }

	private:
		static constexpr int s_FramesInFlight = VulkanAPI::GetMaxFramesInFlight();

		// can use the same for Quads, Circle etc ...
		struct BatchingData
		{
			static const uint32_t s_MaxQuads = 10000;
			static const uint32_t s_MaxVertices = s_MaxQuads * 4;
			static const uint32_t s_MaxIndices = s_MaxQuads * 6;

			std::vector<QuadVertex*> VertexBufferBase;
			QuadVertex* VertexBufferPtr = nullptr;

			SharedPtr<VulkanIndexBuffer> IndexBuffer;
			std::vector<QuadBatch> BatchesArray[s_FramesInFlight]; // vector of QuadBatch for every frame 

			uint32_t CurrentBatch = 0;
			uint32_t IndexCount = 0;

			glm::vec4 QuadVertexDefaultPositions[4] = 
			{
				glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
				glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f),
				glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f),
				glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f)
			};

		} m_QuadBatchingData;


		friend struct QuadBatch;
	};

}

