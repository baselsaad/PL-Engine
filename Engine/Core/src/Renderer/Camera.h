#pragma once


namespace PL_Engine
{

	class Camera
	{
	public:
		Camera(const glm::mat4& projection);
		Camera();
		virtual ~Camera();

		void CalculateProjectionView();
		virtual void OnUpdate(float deltaTime);

		virtual glm::mat4 GetViewProjection() const { return m_Projection; }

		const glm::mat4& GetProjection() const { return m_Projection; }
		void SetProjection(glm::mat4& projection)  { m_Projection = projection; }

	protected:
		glm::mat4 m_Projection;
		float m_OrthographicSize;
		float m_OrthographicNear;
		float m_OrthographicFar;
		float m_AspectRatio;

		glm::vec3 m_Translation;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		glm::vec2 m_LastMousePosition;
	};

}