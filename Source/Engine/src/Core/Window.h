#pragma once
#include <GLFW/glfw3.h>

namespace PAL
{
	class VulkanSwapChain;
	class Event;

	enum class WindowMode
	{
		FullScreen, Windowed
	};

	struct WindowData
	{
		uint32_t Width;
		uint32_t Height;
		std::string Title;
		bool Vsync;
		WindowMode Mode;
	};

	class Window
	{
	public:

		virtual ~Window() {}

		virtual void InitContext() = 0;
		virtual void Close() = 0;

		virtual bool ShouldClose() = 0;
		virtual bool IsVsyncOn() const = 0;

		using EventFunc = std::function<void(Event& e)>;
		virtual void SetupEventCallback(EventFunc&& callback) = 0;
		virtual void Present() = 0;
		virtual void PollEvents() = 0;
		virtual void WaitEvents() = 0;
		virtual void SetVsync(bool enable) = 0;

		virtual void SetScreenMode(WindowMode mode, uint32_t width = 1600, uint32_t height = 900) = 0;
		virtual WindowMode GetWindowMode() = 0;

		virtual void OnResize(int width, int height) = 0;
		virtual void GetFramebufferSize(int& width, int& height) = 0;

		virtual GLFWwindow* GetWindowHandle() = 0;
		virtual const GLFWwindow* GetWindowHandle() const = 0;
		virtual operator GLFWwindow* () const = 0;

		virtual uint32_t GetWindowWidth() const = 0;
		virtual uint32_t GetWindowHeight() const = 0;
		virtual float GetAspectRatio() const = 0;

		virtual const SharedPtr<VulkanSwapChain>& GetSwapChain() const = 0;
		virtual uint32_t GetCurrentFrame() const = 0;
	};

}