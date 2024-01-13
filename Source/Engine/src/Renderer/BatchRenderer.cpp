#include "pch.h"
#include "BatchRenderer.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "RuntimeRenderer.h"
#include "Utilities/Timer.h"

namespace PAL
{
	BatchRenderer::BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer)
	{
		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();

		m_QuadBatchingData.VertexBufferBase.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			m_QuadBatchingData.VertexBufferBase[i] = new QuadVertex[m_QuadBatchingData.s_MaxVertices];

			QuadBatch defaultBatch(i);
			m_QuadBatchingData.BatchesArray[i].push_back(defaultBatch);
			m_QuadBatchingData.CurrentBatch = 0;
		}

		uint32_t* quadIndices = new uint32_t[m_QuadBatchingData.s_MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_QuadBatchingData.s_MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		m_QuadBatchingData.IndexBuffer = NewShared<VulkanIndexBuffer>(cmBuffer, quadIndices, m_QuadBatchingData.s_MaxIndices * sizeof(uint32_t));
		delete[] quadIndices;
	}

	BatchRenderer::~BatchRenderer()
	{
		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();

		for (int frameIndex = 0; frameIndex < framesInFlight; frameIndex++)
		{
			std::vector<QuadBatch>& batches = m_QuadBatchingData.BatchesArray[frameIndex]; // Get Array of batches for this frame 

			for (QuadBatch& batch : batches)
			{
				batch.VertexBuffer->DestroyBuffer();
			}

			delete[] m_QuadBatchingData.VertexBufferBase[frameIndex];
		}

		m_QuadBatchingData.IndexBuffer->DestroyBuffer();
	}

	void BatchRenderer::Begin()
	{
		// reset every thing
		m_QuadBatchingData.CurrentBatch = 0;
		m_QuadBatchingData.IndexCount = 0;
		m_QuadBatchingData.VertexBufferPtr = m_QuadBatchingData.VertexBufferBase[Engine::Get()->GetWindow()->GetCurrentFrame()];
	}

	void BatchRenderer::End()
	{
		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();

		if (m_QuadBatchingData.CurrentBatch < m_QuadBatchingData.BatchesArray[currentFrame].size()) // current batch < size --> we do not use other batches
		{
			// we should destroy all buffers we do not need 
			for (int batchIndex = m_QuadBatchingData.CurrentBatch + 1; batchIndex < m_QuadBatchingData.BatchesArray[currentFrame].size(); batchIndex++)
			{
				std::vector<QuadBatch>& currentFrameBatchesArray = m_QuadBatchingData.BatchesArray[currentFrame];
				currentFrameBatchesArray[batchIndex].VertexBuffer->DestroyBuffer();
			}

			// Resize only if necessary
			if (m_QuadBatchingData.BatchesArray[currentFrame].size() > m_QuadBatchingData.CurrentBatch + 1)
			{
				m_QuadBatchingData.BatchesArray[currentFrame].resize(m_QuadBatchingData.CurrentBatch + 1);
			}
		}

		RuntimeRenderer::GetStats().VertexBufferCount = m_QuadBatchingData.BatchesArray[currentFrame].size();
	}

	void BatchRenderer::FindOrCreateNewQuadBatch()
	{
		CORE_PROFILER_FUNC();

		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();

		m_QuadBatchingData.IndexCount = 0;
		m_QuadBatchingData.VertexBufferPtr = m_QuadBatchingData.VertexBufferBase[currentFrame];

		m_QuadBatchingData.CurrentBatch++;
		if (m_QuadBatchingData.CurrentBatch == m_QuadBatchingData.BatchesArray[currentFrame].size()) // current batch == size -> we do not have free batches -> create new 
		{
			QuadBatch newBatch(currentFrame);
			m_QuadBatchingData.BatchesArray[currentFrame].push_back(newBatch);
		}
	}

	void BatchRenderer::AddQuadToBatch(const glm::mat4& transform, const glm::vec4& color)
	{
		CORE_PROFILER_FUNC();

		constexpr size_t quadVertexCount = 4;
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			m_QuadBatchingData.VertexBufferPtr->Pos = transform * m_QuadBatchingData.QuadVertexDefaultPositions[i];
			m_QuadBatchingData.VertexBufferPtr->Color = color;
			m_QuadBatchingData.VertexBufferPtr++;
		}

		m_QuadBatchingData.IndexCount += 6;
	}

	void BatchRenderer::AddQuadToBatch(const glm::vec3& translation, const glm::vec3& scale, const glm::vec4& color)//calculate TransformationMatrix in GPU 
	{
		CORE_PROFILER_FUNC();

		//constexpr size_t quadVertexCount = 4;
		//for (size_t i = 0; i < quadVertexCount; i++)
		//{
		//	m_QuadBatchingData.VertexBufferPtr->Pos = m_QuadBatchingData.QuadVertexDefaultPositions[i];
		//	m_QuadBatchingData.VertexBufferPtr->Color = color;
		//	m_QuadBatchingData.VertexBufferPtr->Translation = translation;
		//	m_QuadBatchingData.VertexBufferPtr->Scale = scale;
		//	m_QuadBatchingData.VertexBufferPtr++;
		//}

		//m_QuadBatchingData.IndexCount += 6;
	}

	void BatchRenderer::BindCurrentQuadBatch()
	{
		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();

		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadBatchingData.VertexBufferPtr - (uint8_t*)m_QuadBatchingData.VertexBufferBase[currentFrame]);

		std::vector<QuadBatch>& currentFrameQuadBatches = m_QuadBatchingData.BatchesArray[currentFrame];//CurrentFrame ArrayOfBatches
		QuadBatch& currentBatch = currentFrameQuadBatches[m_QuadBatchingData.CurrentBatch]; // Current_Batch in the Current_Frame

		currentBatch.VertexBuffer->SetData(m_QuadBatchingData.VertexBufferBase[currentFrame], dataSize);
	}

	const SharedPtr<VulkanVertexBuffer>& BatchRenderer::GetVertexBuffer()
	{
		int currentFrame = Engine::Get()->GetWindow()->GetCurrentFrame();
		return m_QuadBatchingData.BatchesArray[currentFrame].at(m_QuadBatchingData.CurrentBatch).VertexBuffer;
	}

	QuadBatch::QuadBatch(int frameIndex /*= 0*/)
		: FrameIndex(frameIndex)
	{
		VertexBuffer = NewShared<VulkanVertexBuffer>(BatchRenderer::BatchingData::s_MaxVertices * sizeof(QuadVertex));
	}

}
