#pragma once

namespace PAL
{
	class VertexBuffer;
	class IndexBuffer;
	class CommandBuffer;
	struct QuadVertex;

	struct QuadBatch
	{
		static const uint32_t s_MaxQuads = 10000;
		static const uint32_t s_MaxVertices = s_MaxQuads * 4;
		static const uint32_t s_MaxIndices = s_MaxQuads * 6;
		static inline glm::vec4 s_QuadVertexDefaultPositions[4] =
		{
			glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
			glm::vec4(0.5f, -0.5f, 0.0f, 1.0f),
			glm::vec4(0.5f,  0.5f, 0.0f, 1.0f),
			glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f)
		};

		// for debuging
		int FrameIndex = 0;
		int BatchNumber = 0;

		SharedPtr<VertexBuffer> VertexBufferRef;

		QuadBatch(int frameIndex = 0);
	};

	class BatchRenderer
	{
	public:
		BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer);
		~BatchRenderer();

		void Begin();
		void End();
		void AddQuadToBatch(const glm::mat4& transform, const glm::vec4& color);

		//calculate TransformationMatrix in GPU 
		// void AddQuadToBatch(const glm::vec3& translation, const glm::vec3& scale, const glm::vec4& color);

		void BindCurrentQuadBatch();
		void FindOrCreateNewQuadBatch();

		const SharedPtr<VertexBuffer>& GetVertexBuffer();

		inline bool ShouldDrawCurrentBatch() { return m_QuadIndexCount >= QuadBatch::s_MaxIndices; };
		inline const SharedPtr<IndexBuffer>& GetQuadIndexBuffer() { return m_QuadIndexBuffer; }
		inline uint32_t GetQuadIndexCount() { return m_QuadIndexCount; }

	private:
		//---------------------------------------------------------------------------------------------------------
		// Quads
		//---------------------------------------------------------------------------------------------------------
		
		// Data on CPU-mem before uploading to gpu 
		QuadVertex* m_QuadVertexBufferBase = nullptr;
		QuadVertex* m_QuadVertexBufferPtr = nullptr;

		// Vector of batches for every frame (3 frames on flight)
		std::vector<QuadBatch> m_QuadBatchesArray[3];
		SharedPtr<IndexBuffer> m_QuadIndexBuffer;

		uint32_t m_QuadCurrentBatch = 0;
		uint32_t m_QuadIndexCount = 0;
		//---------------------------------------------------------------------------------------------------------
	};

}

