#include "pch.h"
#include "BatchRenderer.h"

#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/RuntimeRenderer.h"

#include "Utilities/Timer.h"
#include "Core/Engine.h"
#include "RenderAPI.h"


namespace PAL
{
	BatchRenderer::BatchRenderer()
	{
		const int framesInFlight = RenderAPIHelper::GetFramesOnFlight();

		m_QuadVertexBuffer = new QuadVertex[QuadBatch::s_MaxVertices];
		for (int i = 0; i < framesInFlight; i++)
		{

			QuadBatch defaultBatch(i);
			m_QuadBatchesArray[i].push_back(defaultBatch);
			m_QuadCurrentBatch = 0;
		}

		uint32_t* quadIndices = new uint32_t[QuadBatch::s_MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < QuadBatch::s_MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		m_QuadIndexBuffer = RenderAPIHelper::CreateIndexBuffer(quadIndices, QuadBatch::s_MaxIndices * sizeof(uint32_t));
		delete[] quadIndices; // After uploading the data to GPU 
	}

	BatchRenderer::~BatchRenderer()
	{
		const int framesInFlight = RenderAPIHelper::GetFramesOnFlight();

		for (int frameIndex = 0; frameIndex < framesInFlight; frameIndex++)
		{
			std::vector<QuadBatch>& batches = m_QuadBatchesArray[frameIndex]; // Get Array of batches for this frame 

			// maybe let the destrctur do this ? 
			for (QuadBatch& batch : batches)
			{
				batch.VertexBufferRef->DestroyBuffer();
			}
		}

		delete[] m_QuadVertexBuffer;
		m_QuadIndexBuffer->DestroyBuffer();
	}

	void BatchRenderer::Begin()
	{
		// reset every thing
		m_QuadCurrentBatch = 0;
		m_QuadIndexCount = 0;
		m_QuadVerticesCount = 0;
	}

	void BatchRenderer::End()
	{
		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();

		if (m_QuadCurrentBatch < m_QuadBatchesArray[currentFrame].size()) // current batch < size --> we do not use other batches
		{
			// we should destroy all buffers we do not need 
			// this is important, so we can be sure that we have vertexbuffers only for the vertcies we need to render in the next frame
			for (int batchIndex = m_QuadCurrentBatch + 1; batchIndex < m_QuadBatchesArray[currentFrame].size(); batchIndex++)
			{
				std::vector<QuadBatch>& currentFramem_QuadBatchesArray = m_QuadBatchesArray[currentFrame];
				currentFramem_QuadBatchesArray[batchIndex].VertexBufferRef->DestroyBuffer();
			}

			// Resize only if necessary
			if (m_QuadBatchesArray[currentFrame].size() > m_QuadCurrentBatch + 1)
			{
				m_QuadBatchesArray[currentFrame].resize(m_QuadCurrentBatch + 1);
			}
		}

		RuntimeRenderer::GetStats().VertexBufferCount = m_QuadBatchesArray[currentFrame].size();
	}

	void BatchRenderer::FindOrCreateNewQuadBatch()
	{
		CORE_PROFILER_FUNC();

		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();

		// reset
		m_QuadIndexCount = 0;
		m_QuadVerticesCount = 0;
		m_QuadCurrentBatch++;

		if (m_QuadCurrentBatch == m_QuadBatchesArray[currentFrame].size()) // current batch == size -> we do not have free batches -> create new 
		{
			QuadBatch newBatch(currentFrame);
			m_QuadBatchesArray[currentFrame].push_back(newBatch);
		}
	}
	 
	void BatchRenderer::AddQuadToBatch(const glm::mat4& transform, const glm::vec4& color, const glm::vec4& objectId)
	{
		CORE_PROFILER_FUNC();

		constexpr size_t quadVertexCount = 4;
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			m_QuadVertexBuffer[m_QuadVerticesCount].Pos = transform * QuadBatch::s_QuadVertexDefaultPositions[i];
			m_QuadVertexBuffer[m_QuadVerticesCount].Color = color;
			m_QuadVertexBuffer[m_QuadVerticesCount].ObjectId = objectId;
			m_QuadVerticesCount++;
		}

		m_QuadIndexCount += 6;
	}

	void BatchRenderer::BindCurrentQuadBatch()
	{
		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();

		uint32_t dataSize = m_QuadVerticesCount * sizeof(QuadVertex);

		// Current_Batch in the Current_Frame
		QuadBatch& currentBatch = m_QuadBatchesArray[currentFrame].at(m_QuadCurrentBatch);
		currentBatch.VertexBufferRef->SetData(m_QuadVertexBuffer , dataSize);
	}

	const SharedPtr<VertexBuffer>& BatchRenderer::GetVertexBuffer()
	{
		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();
		return m_QuadBatchesArray[currentFrame].at(m_QuadCurrentBatch).VertexBufferRef;
	}

	QuadBatch::QuadBatch(int frameIndex /*= 0*/)
		: FrameIndex(frameIndex)
	{
		VertexBufferRef = RenderAPIHelper::CreateVertexBuffer(s_MaxVertices * sizeof(QuadVertex));
	}

}
