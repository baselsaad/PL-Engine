#pragma once

namespace PAL
{
	class VertexBuffer;
	class IndexBuffer;

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
		bool UseDepth = false;
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

	enum class ShaderStage
	{
		SHADER_STAGE_VERTEX_BIT = 0x00000001,
		SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
		SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
		SHADER_STAGE_COMPUTE_BIT = 0x00000020,
	};

	class RenderAPI
	{
	public:
		virtual ~RenderAPI() {}

		virtual void Init(const RenderApiSpec& spec) = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void BeginMainPass() = 0;
		virtual void EndMainPass() = 0;

		virtual void PushConstant(void* data, int size, ShaderStage stage) {}

		virtual void DrawQuad(const SharedPtr<VertexBuffer>& vertexBuffer, const SharedPtr<IndexBuffer>& indexBuffer, uint32_t indexCount, const glm::mat4& projection) = 0; // delete Later

		virtual void WaitForIdle() = 0;
		virtual void ResizeFrameBuffer(uint32_t width = 0, uint32_t height = 0, int frameIndex = -1) = 0;

		virtual void SetVSync(bool vsync) = 0;

		// Final Image from main framebuffer
		virtual void* GetFinalImage(uint32_t index = 0) = 0;
	};


	class RenderAPIHelper
	{
	public:
		static SharedPtr<VertexBuffer> CreateVertexBuffer(uint32_t size);
		static SharedPtr<IndexBuffer> CreateIndexBuffer(void* data, uint32_t size);

		//TODO: convert this to Macro as Config
		static const int RenderAPIHelper::GetFramesOnFlight();
	};
}