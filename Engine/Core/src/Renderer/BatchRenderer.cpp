#include "pch.h"
#include "BatchRenderer.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "Renderer.h"

namespace PL_Engine
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

		Renderer::GetStats().VertexBufferCount++;


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

		m_QuadBatchingData.IndexBuffer = MakeShared<VulkanIndexBuffer>(cmBuffer, quadIndices, m_QuadBatchingData.s_MaxIndices * sizeof(uint32_t));
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
		m_QuadBatchingData.VertexBufferPtr = m_QuadBatchingData.VertexBufferBase[VulkanAPI::GetCurrentFrame()];
	}

	void BatchRenderer::End()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();

		if (m_QuadBatchingData.CurrentBatch < m_QuadBatchingData.BatchesArray[currentFrame].size())
		{
			// we should destroy all buffers we do not need 
			for (int i = m_QuadBatchingData.CurrentBatch + 1; i < m_QuadBatchingData.BatchesArray[currentFrame].size(); i++)
			{
				m_QuadBatchingData.BatchesArray[currentFrame].at(i).VertexBuffer->DestroyBuffer();
				if (currentFrame == 0)
					Renderer::GetStats().VertexBufferCount--;
			}

			m_QuadBatchingData.BatchesArray[currentFrame].resize(m_QuadBatchingData.CurrentBatch + 1);
		}
	}

	void BatchRenderer::FindOrCreateNewQuadBatch()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();

		m_QuadBatchingData.IndexCount = 0;
		m_QuadBatchingData.VertexBufferPtr = m_QuadBatchingData.VertexBufferBase[VulkanAPI::GetCurrentFrame()];

		m_QuadBatchingData.CurrentBatch++;
		if (m_QuadBatchingData.CurrentBatch == m_QuadBatchingData.BatchesArray[currentFrame].size())
		{
			QuadBatch newBatch(currentFrame);
			m_QuadBatchingData.BatchesArray[currentFrame].push_back(newBatch);

			if (currentFrame == 0)
				Renderer::GetStats().VertexBufferCount++;
		}
	}

	void BatchRenderer::AddQuadToBatch(const glm::mat4& transform, const glm::vec3& color)
	{
		constexpr size_t quadVertexCount = 4;
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			m_QuadBatchingData.VertexBufferPtr->Pos = transform * m_QuadBatchingData.QuadVertexDefaultPositions[i];
			m_QuadBatchingData.VertexBufferPtr->Color = color;
			m_QuadBatchingData.VertexBufferPtr++;
		}

		m_QuadBatchingData.IndexCount += 6;
	}

	void BatchRenderer::BindCurrentQuadBatch()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();

		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadBatchingData.VertexBufferPtr - (uint8_t*)m_QuadBatchingData.VertexBufferBase[currentFrame]);
		m_QuadBatchingData.BatchesArray[currentFrame].at(m_QuadBatchingData.CurrentBatch).VertexBuffer->SetData(m_QuadBatchingData.VertexBufferBase[currentFrame], dataSize);
	}

	const SharedPtr<VulkanVertexBuffer>& BatchRenderer::GetVertexBuffer()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		return m_QuadBatchingData.BatchesArray[currentFrame].at(m_QuadBatchingData.CurrentBatch).VertexBuffer;
	}

	QuadBatch::QuadBatch(int frameIndex /*= 0*/)
		: FrameIndex(frameIndex)
	{
		VertexBuffer = MakeShared<VulkanVertexBuffer>(BatchRenderer::BatchingData::s_MaxVertices * sizeof(QuadVertex));
	}

}
