#include "pch.h"
#include "Renderer.h"

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
	PAL::RenderStats Renderer::s_RenderStats;

	void Renderer::Init(RenderAPITarget target)
	{
		CORE_PROFILER_FUNC();

		switch (target)
		{
			case PAL::RenderAPITarget::Vulkan:	m_RenderAPI = NewShared<VulkanAPI>();		break;
			case PAL::RenderAPITarget::Unknown:	ASSERT(false, "");							break;
			
			default:							ASSERT(false, "");							break;
		}

		m_RenderAPI->Init();

		m_BatchRenderer = new BatchRenderer(VulkanContext::GetVulkanDevice()->GetMainCommandBuffer());
	}

	void Renderer::Shutdown()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		delete m_BatchRenderer;
		m_RenderAPI->Shutdown();
	}

	void Renderer::StartFrame()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		//m_Projection = camera.GetModellViewProjection();

		m_RenderAPI->BeginFrame();
		m_BatchRenderer->Begin();
	}

	void Renderer::Flush()
	{
		CORE_PROFILER_FUNC();

		// Quads
		m_BatchRenderer->BindCurrentQuadBatch();
		m_RenderAPI->DrawQuad(m_BatchRenderer->GetVertexBuffer(), m_BatchRenderer->GetIndexBuffer(), m_BatchRenderer->GetIndexCount(), m_Projection);

		s_RenderStats.DrawCalls++;
	}

	void Renderer::EndFrame()
	{
		CORE_PROFILER_FUNC();

		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		Flush();
		m_RenderAPI->EndFrame();
		m_BatchRenderer->End();

		//std::cout << "Quads: " << s_RenderStats.Quads << ", DrawCalls: " << s_RenderStats.DrawCalls 
		//	<< ", VertexBufferCount: " << s_RenderStats.VertexBufferCount << " * 3\n";

		s_RenderStats.Reset();
	}

	void Renderer::FlushDrawCommands()
	{
		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->FlushDrawCommands();
	}

	void Renderer::DrawQuad(const glm::vec3& translation, const glm::vec3& scale, const glm::vec4& color)
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
		//s_BatchRenderer->AddQuadToBatch(translation, scale, color);
		s_RenderStats.Quads++;
	}

	void Renderer::DrawQuad(const TransformComponent& transform, const glm::vec4& color)
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

	void Renderer::RecordCommand(const std::function<void()>& command)
	{
		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->RecordCommand(command);
	}

	void Renderer::WaitForIdle()
	{
		ASSERT(m_RenderAPI != nullptr, "No RenderAPI is Used");

		m_RenderAPI->WaitForIdle();
	}

	void Renderer::ResizeFrameBuffer(bool resize, int width, int height)
	{
		m_RenderAPI->ResizeFrameBuffer(resize, width, height);
	}

	void Renderer::PresentFrame()
	{
		m_RenderAPI->PresentFrame();
	}

}