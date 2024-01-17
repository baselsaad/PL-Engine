#pragma once
#include "Core/Window.h"
#include "Vulkan/SwapChain.h"

namespace PAL
{

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowData& data);
		virtual ~WindowsWindow();

		virtual void InitContext() override;
		virtual void Close() override;

		virtual void SetupEventCallback(EventFunc&& callback) override;
		virtual void Present() override;
		virtual void PollEvents() override;
		virtual void SetVsync(bool enable) override;
		virtual void OnResize(int width, int height) override;

		virtual const SharedPtr<VulkanSwapChain>& GetSwapChain() const override;

		virtual GLFWwindow* GetWindowHandle() override;
		virtual const GLFWwindow* GetWindowHandle() const override;
		virtual bool IsVsyncOn() const override;
		virtual uint32_t GetWindowWidth() const override;
		virtual uint32_t GetWindowHeight() const override;
		virtual float GetAspectRatio() const override;
		virtual uint32_t GetCurrentFrame() const override { return m_SwapChain->GetCurrentFrame(); }

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

		SharedPtr<VulkanSwapChain> m_SwapChain;
	};
}