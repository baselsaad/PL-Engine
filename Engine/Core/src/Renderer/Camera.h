#pragma once

namespace PL_Engine
{
	class EventHandler;
	class MouseScrolledEvent;
	class ResizeWindowEvent;

	class Camera
	{
	public:
		Camera(float aspectRatio);

		virtual void OnUpdate(float deltaTime);

		void RecalculateViewMatrix();

		void SetupInput(EventHandler& e);
		void OnMouseScroll(const MouseScrolledEvent& e);
		void OnResizeWindow(const ResizeWindowEvent& e);

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		float GetRotation() const { return m_CameraRotation; }
		void SetRotation(float rotation) { m_CameraRotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		inline float GetZoom() const { return m_Zoom; }

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		bool m_Rotation;

		float m_Zoom;
		float m_AspectRatio;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;

		float m_CameraMoveSpeed = 5.0f;
		float m_CameraRotationSpeed = 180.0f;
	};

}
