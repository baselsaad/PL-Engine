#pragma once
#include "Core/Window.h"

namespace PL_Engine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowData& data);
		virtual ~WindowsWindow();

		virtual void SwapBuffers() override;
		virtual void PollEvents() override;
		virtual void SetVsync(bool enable) override;
		virtual void UpdateWindowSize(int width, int height) override;

		virtual GLFWwindow* GetWindowHandle() override;
		virtual const GLFWwindow* GetWindowHandle() const override;
		virtual bool IsVsyncOn() const override;
		virtual uint32_t GetWindowWidth() const override;
		virtual uint32_t GetWindowHeight() const override;
		virtual float GetAspectRatio() const override;

		virtual operator GLFWwindow* () const override { return m_Window; }

	private:
		void HandleErrorMessages();

	private:
		GLFWwindow* m_Window;
		WindowData m_WindowData;
	};
}