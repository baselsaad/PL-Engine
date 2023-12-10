#include "pch.h"
#include "OrthographicCamera.h"
#include "Event/Event.h"
#include "Event/Input.h"
#include "Event/EventHandler.h"

namespace PAL
{

	OrthographicCamera::OrthographicCamera(float aspectratio)
		: Camera(aspectratio)
		, m_Zoom(1.0f)
	{
		SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::OnUpdate(float deltaTime)
	{
		if (Input::IsKeyPressed(KeyCode::A))
		{
			m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
		}
		else if (Input::IsKeyPressed(KeyCode::D))
		{
			m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;
		}

		if (Input::IsKeyPressed(KeyCode::W))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;
		}
		else if (Input::IsKeyPressed(KeyCode::S))
		{
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		}

		CalculateViewProjectionMatrix();
	}

	void OrthographicCamera::SetupInput(EventHandler& e)
	{
		e.BindAction(EventType::MouseScrolled, this, &OrthographicCamera::OnMouseScroll);
		e.BindAction(EventType::ResizeWindow, this, &OrthographicCamera::OnResizeWindow);
	}

	void OrthographicCamera::CalculateViewProjectionMatrix()
	{
		glm::mat4 view = glm::lookAt(m_CameraPosition, m_CameraPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		m_ViewMatrix = view;
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::OnMouseScroll(const MouseScrolledEvent& e)
	{
		m_Zoom -= e.GetYOffset() * 0.25f;
		m_Zoom = glm::max(m_Zoom, 0.25f);

		SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	}

	void OrthographicCamera::OnResizeWindow(const ResizeWindowEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}