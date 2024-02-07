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
			, m_CameraPosition(0.0f)
			, m_ViewTransformation(0.0f)
			, m_ProjectionMatrix(0.0f)
			, m_Mvp(0.0f)
		{
		}

		Camera()
		{}

		virtual void OnUpdate(float deltaTime) {}
		virtual void SetupInput(EventHandler& e) {}
		virtual CameraType GetCameraType() const { return CameraType::Unknown; }
		virtual void SetAspectRatio(float aspectRatio) {}
		virtual float GetAspectRatio() { return 0.0f; }
		virtual void SetViewportSize(uint32_t width, uint32_t height) {}

		glm::vec3 ScreenPosToWorld(glm::vec2 screenPos, glm::vec2 viewportSize);

		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const glm::mat4& GetViewMatrix() const { return m_ViewTransformation; }
		inline const glm::mat4& GetModelViewProjection() const { return m_Mvp; }


		inline const glm::vec3& GetPosition() const { return m_CameraPosition; }

	protected:
		float m_AspectRatio = 16.0f / 9.0f;
		glm::vec3 m_CameraPosition;

		// Defines how the 3D world is projected onto a 2D surface (screen)
		glm::mat4 m_ProjectionMatrix;
		// Positions and orients the camera within the 3D scene
		glm::mat4 m_ViewTransformation;
		// Model-View-Projection Combines the view and projection matrices for rendering
		glm::mat4 m_Mvp;
	};

}
