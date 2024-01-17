#pragma once
#include "Camera.h"

namespace PAL
{
	struct MouseMoveEvent;

	class OrthographicCamera : public Camera
	{

	public:
		OrthographicCamera(float aspectratio);

		virtual void OnUpdate(float deltaTime) override;
		virtual void SetupInput(EventHandler& e) override;
		virtual CameraType GetCameraType() const override { return CameraType::Orthographic; }
		virtual void SetAspectRatio(float aspectRatio) override;
		virtual float GetAspectRatio() override { return m_AspectRatio; }

		void CalculateViewProjectionMatrix();
		void OnMouseScroll(const MouseScrolledEvent& e);
		void OnResizeWindow(const ResizeWindowEvent& e);
		void SetOrthoProjectionBounds(float left, float right, float bottom, float top);

		inline float GetZoom() const { return m_Zoom; }

	private:
		float m_Zoom;
		float m_CameraMoveSpeed = 5.0f;
		glm::vec2 m_LastMousePos;
	};

}