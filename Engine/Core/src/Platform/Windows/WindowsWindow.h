#pragma once
#include "Core/Window.h"


namespace PL_Engine
{
	class VulkanSwapChain;

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowData& data);
		virtual ~WindowsWindow();
		virtual void Close() override;

		virtual void SwapBuffers() override;
		virtual void PollEvents() override;
		virtual void SetVsync(bool enable) override;
		virtual void OnResize(int width, int height) override;

		virtual GLFWwindow* GetWindowHandle() override;
		virtual const GLFWwindow* GetWindowHandle() const override;
		virtual bool IsVsyncOn() const override;
		virtual uint32_t GetWindowWidth() const override;
		virtual uint32_t GetWindowHeight() const override;
		virtual float GetAspectRatio() const override;

		virtual operator GLFWwindow* () const override { return m_WindowHandle; }

		virtual void WaitEvents() override;
		virtual void GetFramebufferSize(int& width, int& height) override;

		virtual bool ShouldClose() override;

	private:
		void HandleErrorMessages();

	private:
		GLFWwindow* m_WindowHandle;
		WindowData m_WindowData;
	};
}