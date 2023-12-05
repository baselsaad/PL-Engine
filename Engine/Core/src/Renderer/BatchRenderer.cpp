#include "pch.h"
#include "BatchRenderer.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/CommandBuffer.h"

namespace PL_Engine
{
	BatchRenderer::BatchRenderer(const SharedPtr<CommandBuffer>& cmBuffer)
	{
		int framesInFlight = VulkanAPI::GetMaxFramesInFlight();

		m_QuadVertexBuffer.resize(framesInFlight);
		m_QuadVertexBufferBase.resize(framesInFlight);

		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			m_QuadVertexBuffer[i] = MakeShared<VulkanVertexBuffer>(s_MaxVertices * sizeof(QuadVertex));
			m_QuadVertexBufferBase[i] = new QuadVertex[s_MaxVertices];
		}

		m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
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
		int framesInFlight = VulkanAPI::GetMaxFramesInFlight();

		for (int i = 0; i < framesInFlight; i++)
		{
			m_QuadVertexBuffer[i]->DestroyBuffer();
			delete[] m_QuadVertexBufferBase[i];
		}

		m_QuadIndexBuffer->DestroyBuffer();
	}

	void BatchRenderer::StartNewBatch()
	{
		m_QuadIndexCount = 0;
		m_QuadVertexBufferPtr = m_QuadVertexBufferBase[VulkanAPI::GetCurrentFrame()];
	}

	void BatchRenderer::BatchNewQuadVertices(const glm::mat4& transform, const glm::vec3& color)
	{
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
		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadVertexBufferPtr - (uint8_t*)m_QuadVertexBufferBase[currentFrame]);

		m_QuadVertexBuffer[currentFrame]->SetData(m_QuadVertexBufferBase[currentFrame], dataSize);
	}
}