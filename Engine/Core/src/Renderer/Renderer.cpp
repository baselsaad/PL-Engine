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
	SharedPtr<IRenderAPI> Renderer::s_RenderAPI;
	BatchRenderer* Renderer::s_BatchRenderer;
	glm::mat4 Renderer::s_Projection;
	PAL::RenderStats Renderer::s_RenderStats;

	void Renderer::Init(RenderAPITarget target)
	{
		SCOPE_TIMER("Init Renderer");

		switch (target)
		{
			case PAL::RenderAPITarget::Vulkan:	s_RenderAPI = MakeShared<VulkanAPI>();		break;
			case PAL::RenderAPITarget::Unknown:	ASSERT(false, "");							break;
			default:							ASSERT(false, "");							break;
		}

		s_RenderAPI->InitRenderApiContext();
		s_RenderAPI->Init();
		VulkanMemoryAllocator::Init(VulkanContext::GetVulkanDevice());

		//@TODO: Move later GetCommandBuffer
		auto cmdBuffer = static_cast<VulkanAPI*>((VulkanAPI*)s_RenderAPI.get())->GetCommandBuffer();
		s_BatchRenderer = new BatchRenderer(cmdBuffer);
	}

	void Renderer::Shutdown()
	{
		SCOPE_TIMER("Renderer Renderer");
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		delete s_BatchRenderer;
		s_RenderAPI->Shutdown();
	}

	void Renderer::StartFrame(const Camera& camera)
	{
		SCOPE_TIMER();

		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_Projection = camera.GetModellViewProjection();

		s_RenderAPI->BeginFrame();
		s_BatchRenderer->Begin();
	}

	void Renderer::Flush()
	{
		SCOPE_TIMER();

		// Quads
		s_BatchRenderer->BindCurrentQuadBatch();
		s_RenderAPI->DrawQuad(s_BatchRenderer->GetVertexBuffer(), s_BatchRenderer->GetIndexBuffer(), s_BatchRenderer->GetIndexCount(), s_Projection);

		s_RenderStats.DrawCalls++;
	}

	void Renderer::EndFrame()
	{
		SCOPE_TIMER();

		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		Flush();
		s_RenderAPI->EndFrame();
		s_BatchRenderer->End();

		//std::cout << "Quads: " << s_RenderStats.Quads << ", DrawCalls: " << s_RenderStats.DrawCalls 
		//	<< ", VertexBufferCount: " << s_RenderStats.VertexBufferCount << " * 3\n";

		s_RenderStats.Reset();
	}

	void Renderer::DrawQuad(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& color)
	{
		SCOPE_TIMER();

		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");
		if (s_BatchRenderer->ShouldDrawCurrentBatch())
		{
			Flush();
			s_BatchRenderer->FindOrCreateNewQuadBatch();
		}

		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), translation)
			* glm::mat4(1.0f)
			* glm::scale(glm::mat4(1.0f), scale);

		s_BatchRenderer->AddQuadToBatch(transform, color);
		//s_BatchRenderer->AddQuadToBatch(translation, scale, color);
		s_RenderStats.Quads++;
	}

	void Renderer::DrawQuad(const TransformComponent& transform, const glm::vec3& color)
	{
		SCOPE_TIMER();

		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");
		if (s_BatchRenderer->ShouldDrawCurrentBatch())
		{
			Flush();
			s_BatchRenderer->FindOrCreateNewQuadBatch();
		}

		s_BatchRenderer->AddQuadToBatch(transform.GetTransformMatrix(), color);
		s_RenderStats.Quads++;
	}

	void Renderer::RecordCommand(const std::function<void()>& command)
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->RecordCommand(command);
	}

	void Renderer::WaitForIdle()
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->WaitForIdle();
	}

	void Renderer::OnResizeWindow(bool resize, int width, int height)
	{
		s_RenderAPI->OnResizeWindow(resize);
	}
}