#pragma once

namespace PAL
{
	class EventHandler;
	class MouseScrolledEvent;
	class ResizeWindowEvent;

	enum class CameraType
	{
		Orthographic, Perspective, Unknown
	};

	class Camera
	{
	public:
		Camera(float aspectRatio)
			: m_AspectRatio(aspectRatio)
			, m_ViewMatrix(1.0f)
		{
		}

		virtual void OnUpdate(float deltaTime) {}
		virtual void SetupInput(EventHandler& e) {}
		virtual CameraType GetCameraType() const { return CameraType::Unknown; }

		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

		inline const glm::vec3& GetPosition() const { return m_CameraPosition; }


	protected:
		float m_AspectRatio;
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };

		// Defines how the 3D world is projected onto a 2D surface (screen)
		glm::mat4 m_ProjectionMatrix;
		// Positions and orients the camera within the 3D scene
		glm::mat4 m_ViewMatrix;
		// Combines the view and projection matrices for rendering
		glm::mat4 m_ViewProjectionMatrix;
	};

}
