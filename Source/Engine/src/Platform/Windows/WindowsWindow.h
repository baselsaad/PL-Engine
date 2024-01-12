#pragma once
#include "Core/Window.h"


namespace PAL
{
	class VulkanSwapChain;

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowData& data);
		virtual ~WindowsWindow();
		virtual void Close() override;

		virtual void SetupEventCallback(EventFunc&& callback) override;
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
		virtual void SetScreenMode(WindowMode mode, uint32_t width = 1600, uint32_t height = 900) override;
		virtual WindowMode GetWindowMode() override { return m_WindowData.Mode; }

		virtual bool ShouldClose() override;

	private:
		void HandleErrorMessages();

	private:
		GLFWwindow* m_WindowHandle;
		WindowData m_WindowData;

		EventFunc m_EventCallback;
	};
}