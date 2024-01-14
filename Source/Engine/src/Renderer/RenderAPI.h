#pragma once

namespace PAL
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	enum class PresentTarget
	{
		Swapchain,
		CustomViewport
	};

	struct FramebufferSpecification
	{
		VkFormat ColorFormat;
		VkFormat DepthFormat;
		uint32_t Width;
		uint32_t Height;
		bool UseDepth;
		PresentTarget Target;	// Indicates if the framebuffer is for swapchain images

		std::string DebugName;
	};

	struct RenderpassSpecification
	{
		VkFormat ColorFormat;
		VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
		bool UseDepth;
		PresentTarget Target; // Indicates if the framebuffer is for swapchain images

		std::string DebugName;
	};

	struct RenderApiSpec
	{
		RenderpassSpecification  MainRenderpassSpec = {};  // Main Scene FrameBuffer
		FramebufferSpecification MainFrameBufferSpec = {}; // Main Scene Renderpass
	};

	class IRenderAPI
	{
	public:
		virtual ~IRenderAPI() {}

		virtual void Init(const RenderApiSpec& spec) = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void BeginMainPass() = 0;
		virtual void EndMainPass() = 0;

		virtual void DrawQuad(const SharedPtr<VulkanVertexBuffer>& vertexBuffer, const SharedPtr<VulkanIndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection) = 0; // delete Later

		virtual void WaitForIdle() = 0;
		virtual void ResizeFrameBuffer(bool resize = false, uint32_t width = 0, uint32_t height = 0) = 0;

		virtual void SetVSync(bool vsync) = 0;

		// Final Image from main framebuffer
		virtual void* GetFinalImage(uint32_t index = 0) = 0;
	};
}