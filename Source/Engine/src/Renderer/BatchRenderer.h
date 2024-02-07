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

		/*
		* -0,5(3) 			0,5(2)
		*
		*
		* -0,5(0)			0,5(1)
		*/

		static glm::vec4 CalculateAABBMin(glm::mat4& tranMatrix)
		{
			return tranMatrix * s_QuadVertexDefaultPositions[0];
		}

		static glm::vec4 CalculateAABBMax(glm::mat4& tranMatrix)
		{
			//glm::vec3 maxPoint(-FLT_MAX);
			//for (int i = 0; i < 4; ++i)
			//{
			//	auto copy = tranMatrix * s_QuadVertexDefaultPositions[i];

			//	maxPoint.x = glm::max(maxPoint.x, copy.x);
			//	maxPoint.y = glm::max(maxPoint.y, copy.y);
			//	maxPoint.z = glm::max(maxPoint.z, copy.z);
			//}
			//return maxPoint;

			return tranMatrix * s_QuadVertexDefaultPositions[2];
		}


		// for debuging
		int FrameIndex = 0;
		int BatchNumber = 0;

		SharedPtr<VertexBuffer> VertexBufferRef;

		QuadBatch(int frameIndex = 0);
	};

	class BatchRenderer
	{
	public:
		BatchRenderer();
		~BatchRenderer();

		void Begin();
		void End();
		void AddQuadToBatch(const glm::mat4& transform, const glm::vec4& color);

		//calculate TransformationMatrix in GPU 
		// void AddQuadToBatch(const glm::vec3& translation, const glm::vec3& scale, const glm::vec4& color);

		// Upload the data to GPU vertexbuffer 
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
		QuadVertex* m_QuadVertexBuffer = nullptr;
		uint32_t m_QuadVerticesCount = 0;

		// Vector of batches for every frame (3 frames on flight)
		std::vector<QuadBatch> m_QuadBatchesArray[3];
		SharedPtr<IndexBuffer> m_QuadIndexBuffer;

		uint32_t m_QuadCurrentBatch = 0;
		uint32_t m_QuadIndexCount = 0;
		//---------------------------------------------------------------------------------------------------------
	};

}

