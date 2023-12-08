#include "pch.h"
#include "Renderer.h"

#include "RenderAPI.h"
#include "Vulkan/VulkanRenderer.h"
#include "Core/Engine.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VertexBuffer.h"
#include "Utilities/Colors.h"
#include "EditorCamera.h"

namespace PL_Engine
{
	SharedPtr<IRenderAPI> Renderer::s_RenderAPI;
	BatchRenderer* Renderer::s_BatchRenderer;
	glm::mat4 Renderer::s_Projection;
	PL_Engine::RenderStats Renderer::s_RenderStats;

	void Renderer::Init(RenderAPITarget target)
	{
		switch (target)
		{
			case PL_Engine::RenderAPITarget::Vulkan:	s_RenderAPI = MakeShared<VulkanAPI>();		break;
			case PL_Engine::RenderAPITarget::Unknown:	ASSERT(false, "");							break;
			default:									ASSERT(false, "");							break;
		}

		s_RenderAPI->InitRenderApiContext();
		VulkanMemoryAllocator::Init(VulkanContext::GetVulkanDevice());

		s_RenderAPI->Init();

		//@TODO: Move later GetCommandBuffer
		auto cmdBuffer = static_cast<VulkanAPI*>((VulkanAPI*)s_RenderAPI.get())->GetCommandBuffer();
		s_BatchRenderer = new BatchRenderer(cmdBuffer);
	}

	void Renderer::Shutdown()
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		delete s_BatchRenderer;
		s_RenderAPI->Shutdown();
	}

	void Renderer::BeginFrame(const Camera& camera)
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_Projection = camera.GetViewProjection();

		s_RenderAPI->BeginFrame();
		s_BatchRenderer->Begin();
	}

	void Renderer::Flush()
	{
		int currentFrame = VulkanAPI::GetCurrentFrame();
		
		// Quads
		s_BatchRenderer->BindCurrentBatch();
		s_RenderAPI->DrawQuad(s_BatchRenderer->GetVertexBuffer(), s_BatchRenderer->GetIndexBuffer(), s_BatchRenderer->GetIndexCount(), s_Projection);

		s_RenderStats.DrawCalls++;
	}

	void Renderer::EndFrame()
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		Flush();
		s_RenderAPI->EndFrame();
		s_BatchRenderer->End();

		s_RenderStats.Reset();
	}

	void Renderer::DrawQuad(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& color)
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		if (s_BatchRenderer->IsBatchFull())
		{
			Flush();
			s_BatchRenderer->FindOrCreateNewBatch();
		}

		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), translation)
			* glm::mat4(1.0f)
			* glm::scale(glm::mat4(1.0f), scale);

		s_BatchRenderer->BatchNewQuadVertices(transform, color);
		s_RenderStats.Quads++;
	}

	void Renderer::SubmitCommand(const std::function<void()>& command)
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->SubmitCommand(command);
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