#include "pch.h"
#include "RuntimeRenderer.h"

#include "RenderAPI.h"
#include "Core/Engine.h"

// Move to one .h
#include "Vulkan/VulkanRenderer.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanContext.h"

#include "VertexBuffer.h"
#include "Utilities/Colors.h"
#include "Utilities/Timer.h"
#include "Map/ECS.h"

namespace PAL
{
	PAL::RenderStats RuntimeRenderer::s_RenderStats;

	void RuntimeRenderer::Init(const RuntimeRendererSpecification& spec)
	{
		CORE_PROFILER_FUNC();

		m_RuntimeRendererSpecification = spec;

		switch (spec.TargetAPI)
		{
			case PAL::RenderAPITarget::Vulkan:	m_RenderAPI = NewShared<VulkanAPI>(); break;
			case PAL::RenderAPITarget::Unknown:	PAL_ASSERT(false, "");                break;
		}

		m_RenderAPI->Init(spec.ApiSpec);
		m_BatchRenderer = new BatchRenderer();
	}

	void RuntimeRenderer::Shutdown()
	{
		CORE_PROFILER_FUNC();

		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		delete m_BatchRenderer;
		m_RenderAPI->Shutdown();
	}

	void RuntimeRenderer::StartFrame()
	{
		CORE_PROFILER_FUNC();

		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->BeginFrame();
		m_BatchRenderer->Begin();
	}

	void RuntimeRenderer::DrawBatch()
	{
		CORE_PROFILER_FUNC();

		// Quads
		{
			m_BatchRenderer->BindCurrentQuadBatch();

			const auto& vertexBuffer = m_BatchRenderer->GetVertexBuffer();
			uint32_t indexCount = m_BatchRenderer->GetQuadIndexCount();

			m_DrawCommandsQueue.Record([this, vertexBuffer, indexCount] ()
			{
				m_RenderAPI->DrawQuad(vertexBuffer, m_BatchRenderer->GetQuadIndexBuffer(), indexCount, m_Projection);
			});
		}
	}

	void RuntimeRenderer::EndFrame()
	{
		CORE_PROFILER_FUNC();

		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		DrawBatch();
		m_RenderAPI->EndFrame();
		m_BatchRenderer->End();
	}

	void RuntimeRenderer::FlushDrawCommands()
	{
		CORE_PROFILER_FUNC();

		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderStats.DrawCalls = m_DrawCommandsQueue.GetCommandsCount();
		s_RenderStats.DrawCommandsQueueUsage = m_DrawCommandsQueue.GetCommandsUsage();

		m_RenderAPI->BeginMainPass();
		{
			m_DrawCommandsQueue.ExecuteAndClear();
		}
		m_RenderAPI->EndMainPass();
	}

	void RuntimeRenderer::DrawQuad(const TransformComponent& transform, const glm::vec4& color, uint32_t entityID /* -1 */)
	{
		DrawQuad(transform.GetTransformMatrix(), color, entityID);
	}

	void RuntimeRenderer::DrawQuad(const glm::mat4 transformationMatrix, const glm::vec4& color, uint32_t entityID)
	{
		CORE_PROFILER_FUNC();

		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");
		if (m_BatchRenderer->ShouldDrawCurrentBatch())
		{
			DrawBatch();
			m_BatchRenderer->FindOrCreateNewQuadBatch();
		}

		glm::vec4 objectID; 
		objectID.a = 1.0f;
		objectID.b = static_cast<float>((entityID >> 16) & 0xFF) / 255.0f;
		objectID.g = static_cast<float>((entityID >> 8) & 0xFF) / 255.0f;
		objectID.r = static_cast<float>((entityID >> 0) & 0xFF) / 255.0f;

		m_BatchRenderer->AddQuadToBatch(transformationMatrix, color, objectID);
		s_RenderStats.Quads++;
	}

	void RuntimeRenderer::WaitForIdle()
	{
		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->WaitForIdle();
	}

	void RuntimeRenderer::ResizeFrameBuffer(uint32_t width, uint32_t height, int frameIndex)
	{
		PAL_ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->ResizeFrameBuffer(width, height, frameIndex);
	}

	void* RuntimeRenderer::GetFinalImage(uint32_t index /*= 0*/)
	{
		return m_RenderAPI->GetFinalImage(index);
	}

	void RuntimeRenderer::SetVSync(bool vsync)
	{
		m_RenderAPI->SetVSync(vsync);
	}

}