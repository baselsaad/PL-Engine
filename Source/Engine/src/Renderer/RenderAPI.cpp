#include "pch.h"
#include "RenderAPI.h"

#include "Core/Engine.h"
#include "Renderer/RuntimeRenderer.h"
#include "Utilities/Debug.h"

#include "Vulkan/VulkanVertexBuffer.h"
#include "Vulkan/VulkanIndexBuffer.h"
#include "Vulkan/VulkanRenderer.h"


namespace PAL
{
	SharedPtr<VertexBuffer> RenderAPIHelper::CreateVertexBuffer(uint32_t size)
	{
		RenderAPITarget target = Engine::Get()->GetCurrentRenderAPI();

		switch (target)
		{
			case RenderAPITarget::Vulkan:  return NewShared<VulkanVertexBuffer>(size);
			case RenderAPITarget::Unknown: PAL_ASSERT(false, "");  return nullptr;
		}
	}

	SharedPtr<IndexBuffer> RenderAPIHelper::CreateIndexBuffer(void* data, uint32_t size)
	{
		RenderAPITarget target = Engine::Get()->GetCurrentRenderAPI();

		switch (target)
		{
			case RenderAPITarget::Vulkan:  return NewShared<VulkanIndexBuffer>(data, size);
			case RenderAPITarget::Unknown: PAL_ASSERT(false, "");  return nullptr;
		}
	}

	const int RenderAPIHelper::GetFramesOnFlight()
	{
		return VulkanAPI::GetMaxFramesInFlight();
	}

}