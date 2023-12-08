#include "pch.h"
#include "BatchRenderer.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/VulkanContext.h"



namespace PL_Engine
{
	BatchRenderer::BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer)
	{
		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();

		m_QuadVertexBufferBase.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			m_QuadVertexBufferBase[i] = new QuadVertex[QuadBatch::s_MaxVertices];
			
			QuadBatch batch(i);
			m_Batches[i].push_back(batch);
			m_CurrentQuadBatch[i] = 0;
		}

		m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		uint32_t* quadIndices = new uint32_t[s_MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		m_QuadIndexBuffer = MakeShared<VulkanIndexBuffer>(cmBuffer, quadIndices, s_MaxIndices * sizeof(uint32_t));
		delete[] quadIndices;
	}

	BatchRenderer::~BatchRenderer()
	{
		constexpr int framesInFlight = VulkanAPI::GetMaxFramesInFlight();
		
		for (int i = 0; i < framesInFlight; i++)
		{
			for (QuadBatch& batch : m_Batches[i])
			{
				batch.VertexBuffer->DestroyBuffer();
			}

			delete[] m_QuadVertexBufferBase[i];
		}

		m_QuadIndexBuffer->DestroyBuffer();
	}

	void BatchRenderer::Begin()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		m_CurrentQuadBatch[currentFrame] = 0;
		FindOrCreateNewBatch();
	}

	void BatchRenderer::End()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		for (QuadBatch& batch : m_Batches[currentFrame])
		{
			batch.IsFull = false;
		}
	}

	void BatchRenderer::FindOrCreateNewBatch()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		int& currentBatch = m_CurrentQuadBatch[currentFrame];

		m_QuadIndexCount = 0;
		m_QuadVertexBufferPtr = m_QuadVertexBufferBase[VulkanAPI::GetCurrentFrame()];
		
		auto& batches = m_Batches[currentFrame];
		for (int i = 0; i < batches.size(); i++)
		{
			QuadBatch& batch = batches[i];
			if (batch.IsFull)
				continue;

			currentBatch = i;
			return;
		}

		QuadBatch newBatch(currentFrame);
		m_Batches[currentFrame].push_back(newBatch);
	}

	void BatchRenderer::BatchNewQuadVertices(const glm::mat4& transform, const glm::vec3& color)
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		int currentBatch = m_CurrentQuadBatch[currentFrame];

		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			m_QuadVertexBufferPtr->Pos = transform * m_QuadVertexPositions[i];
			m_QuadVertexBufferPtr->Color = color;
			m_QuadVertexBufferPtr++;
		}

		m_QuadIndexCount += 6;
	}

	void BatchRenderer::BindCurrentBatch()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		int currentBatch = m_CurrentQuadBatch[currentFrame];

		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadVertexBufferPtr - (uint8_t*)m_QuadVertexBufferBase[currentFrame]);

		m_Batches[currentFrame].at(currentBatch).VertexBuffer->SetData(m_QuadVertexBufferBase[currentFrame], dataSize);
		
		// This function get called only when the batch is full and ready to rendered 
		m_Batches[currentFrame].at(currentBatch).IsFull = true;
	}

	const SharedPtr<VulkanVertexBuffer>& BatchRenderer::GetVertexBuffer()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		int currentBatch = m_CurrentQuadBatch[currentFrame];

		return m_Batches[currentFrame].at(currentBatch).VertexBuffer;
	}

}
