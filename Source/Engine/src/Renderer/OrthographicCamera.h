#pragma once
#include "Camera.h"

namespace PAL
{
	class MouseMoveEvent;

	class OrthographicCamera : public Camera
	{

	public:
		OrthographicCamera(float size, float nearClip, float farClip);

		virtual void OnUpdate(float deltaTime) override;
		virtual void SetupInput(EventHandler& e) override;
		virtual CameraType GetCameraType() const override { return CameraType::Orthographic; }
		virtual float GetAspectRatio() override { return m_AspectRatio; }

		void CalculateViewProjectionMatrix();
		void OnMouseScroll(const MouseScrolledEvent& e);
		void OnResizeWindow(const ResizeWindowEvent& e);
		void SetOrthoProjectionBounds(const float width, const float height, const float nearP, const float farP);

		virtual void SetViewportSize(uint32_t width, uint32_t height) override;

		inline float GetOrthographicSize() const { return m_OrthographicSize; }

	private:
		float m_CameraMoveSpeed = 5.0f;
		glm::vec2 m_LastMousePos;
		
		float m_OrthographicSize = 5.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;
	};

}