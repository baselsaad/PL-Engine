#include "pch.h"
#include "RuntimeRenderer.h"

#include "RenderAPI.h"
#include "Vulkan/VulkanRenderer.h"
#include "Core/Engine.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VertexBuffer.h"
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
			case PAL::RenderAPITarget::Vulkan:	m_RenderAPI = NewShared<VulkanAPI>();		break;
			case PAL::RenderAPITarget::Unknown:	ASSERT(false, "");							break;

			default:							ASSERT(false, "");							break;
		}

		m_RenderAPI->Init(spec.ApiSpec);
		m_BatchRenderer = new BatchRenderer(VulkanContext::GetVulkanDevice()->GetMainCommandBuffer());
	}

	void RuntimeRenderer::Shutdown()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		delete m_BatchRenderer;
		m_RenderAPI->Shutdown();
	}

	void RuntimeRenderer::StartFrame()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->BeginFrame();
		m_BatchRenderer->Begin();
	}

	void RuntimeRenderer::Flush()
	{
		CORE_PROFILER_FUNC();

		// Quads
		m_BatchRenderer->BindCurrentQuadBatch();

		const auto& vertexBuffer = m_BatchRenderer->GetVertexBuffer();
		uint32_t indexCount = m_BatchRenderer->GetIndexCount();

		m_DrawCommandsQueue.Record([this, vertexBuffer, indexCount]()
		{
			m_RenderAPI->DrawQuad(vertexBuffer, m_BatchRenderer->GetIndexBuffer(), indexCount, m_Projection);
		});
	}

	void RuntimeRenderer::EndFrame()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		Flush();
		m_RenderAPI->EndFrame();
		m_BatchRenderer->End();
	}

	void RuntimeRenderer::FlushDrawCommands()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderStats.DrawCalls = m_DrawCommandsQueue.GetCommandsCount();
		s_RenderStats.DrawCommandsQueueUsage = m_DrawCommandsQueue.GetCommandsUsage();

		m_RenderAPI->BeginMainPass();
		{
			m_DrawCommandsQueue.ExecuteAll();
			m_DrawCommandsQueue.Clear();
		}
		m_RenderAPI->EndMainPass();
	}

	void RuntimeRenderer::DrawQuad(const glm::vec3& translation, const glm::vec3& scale, const glm::vec4& color)
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");
		if (m_BatchRenderer->ShouldDrawCurrentBatch())
		{
			Flush();
			m_BatchRenderer->FindOrCreateNewQuadBatch();
		}

		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), translation)
			* glm::mat4(1.0f)
			* glm::scale(glm::mat4(1.0f), scale);

		m_BatchRenderer->AddQuadToBatch(transform, color);
		s_RenderStats.Quads++;
	}

	void RuntimeRenderer::DrawQuad(const TransformComponent& transform, const glm::vec4& color)
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");
		if (m_BatchRenderer->ShouldDrawCurrentBatch())
		{
			Flush();
			m_BatchRenderer->FindOrCreateNewQuadBatch();
		}

		m_BatchRenderer->AddQuadToBatch(transform.GetTransformMatrix(), color);
		s_RenderStats.Quads++;
	}

	void RuntimeRenderer::WaitForIdle()
	{
		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->WaitForIdle();
	}

	void RuntimeRenderer::ResizeFrameBuffer(bool resize, uint32_t width, uint32_t height)
	{
		m_RenderAPI->ResizeFrameBuffer(resize, width, height);
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