#include "pch.h"
#include "Renderer.h"

#include "RenderAPI.h"
#include "Vulkan/VulkanRenderer.h"
#include "Core/Engine.h"
#include "Vulkan/SwapChain.h"

namespace PL_Engine
{
	SharedPtr<IRenderAPI> Renderer::s_RenderAPI;

	void Renderer::Init(RenderAPITarget target)
	{
		switch (target)
		{
			case PL_Engine::RenderAPITarget::Vulkan:	s_RenderAPI = MakeShared<VulkanAPI>();		break;
			case PL_Engine::RenderAPITarget::Unknown:	ASSERT(false, "");							break;
			default:									ASSERT(false, "");							break;
		}

		s_RenderAPI->InitRenderApiContext();
		s_RenderAPI->Init();
	}

	void Renderer::Shutdown()
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->Shutdown();
	}

	void Renderer::BeginFrame()
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->EndFrame();
	}

	void Renderer::DrawQuad(const glm::vec3& translation)
	{
		ASSERT(s_RenderAPI != nullptr, "No RenderAPI is Used");

		s_RenderAPI->DrawQuad(translation);
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