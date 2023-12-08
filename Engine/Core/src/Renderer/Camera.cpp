#include "pch.h"
#include "Camera.h"
#include "Core/Engine.h"
#include "Platform/Windows/WindowsWindow.h"

#include "glm/gtx/quaternion.hpp"
#include "Event/Input.h"
#include "Utilities/Log.h"


namespace PL_Engine
{
	static bool s_Pressed = false;
	static glm::vec3 s_MousePressedPos = glm::vec3(0.0f);
	static glm::vec3 s_DeltaMouse = glm::vec3(0.0f);

	Camera::Camera(const glm::mat4& projection)
		: m_Projection(projection)
		, m_OrthographicSize(10.0f)
		, m_OrthographicNear(-1.0f)
		, m_OrthographicFar(1.0f)
		, m_AspectRatio(Engine::Get()->GetWindow()->GetAspectRatio())
		, m_Translation(glm::vec3(0.0f)), m_Rotation(0.0f), m_Scale(glm::vec3(1.0f))
		, m_LastMousePosition(glm::vec2(0.0f))
	{
	}

	Camera::Camera()
		: Camera(glm::mat4(1.0f))
	{
		CalculateProjectionView();
	}

	Camera::~Camera()
	{
	}

	void Camera::CalculateProjectionView()
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));

		glm::mat4 cameraTransform = glm::translate(glm::mat4(1.0f), m_Translation)
			* rotation
			* glm::scale(glm::mat4(1.0f), m_Scale);

		// TODO: Get values
		//m_OrthographicSize = 10.0f;
		//m_OrthographicNear = -1.0f;
		//m_OrthographicFar = 1.0f;
		m_AspectRatio = Engine::Get()->GetWindow()->GetAspectRatio();

		float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoBottom = -m_OrthographicSize * 0.5f;
		float orthoTop = m_OrthographicSize * 0.5f;

		glm::mat4 projection = glm::ortho(orthoLeft, orthoRight,
			orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);

		m_Projection = projection * glm::inverse(cameraTransform);
	}

	void Camera::OnUpdate(float deltaTime)
	{
		if (Input::IsKeyPressed(KeyCode::LeftAlt))
		{
			glm::vec2 mouse = Input::GetMousePosition();
			glm::vec2 delta = (m_LastMousePosition - mouse) * (0.001f * deltaTime);
			m_LastMousePosition = mouse;

			if (Input::IsMouseButtonDown(MouseButtonKey::Left))
			{
				m_Translation.x += (delta.x);
				m_Translation.y += (delta.y * -1.0f);
			}
		}

		CalculateProjectionView();
	}

}
