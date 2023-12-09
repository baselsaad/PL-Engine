#include "pch.h"
#include "Camera.h"
#include "Core/Engine.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Event/Event.h"
#include "Event/EventHandler.h"
#include "Event/Input.h"

namespace PL_Engine
{
	Camera::Camera(float aspectRatio)
		: m_AspectRatio(aspectRatio)
		, m_ViewMatrix(1.0f)
		, m_Zoom(1.0f)
		, m_Rotation(true)
	{
		SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera::OnUpdate(float deltaTime)
	{
		if (Input::IsKeyPressed(KeyCode::A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
		}
		else if (Input::IsKeyPressed(KeyCode::D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
		}

		if (Input::IsKeyPressed(KeyCode::W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
		}
		else if (Input::IsKeyPressed(KeyCode::S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * deltaTime;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(KeyCode::Q))
				m_CameraRotation += m_CameraRotationSpeed * deltaTime;
			if (Input::IsKeyPressed(KeyCode::E))
				m_CameraRotation -= m_CameraRotationSpeed * deltaTime;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			SetRotation(m_CameraRotation);
		}

		SetPosition(m_CameraPosition);

		m_CameraMoveSpeed = m_Zoom;
	}

	void Camera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_CameraRotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera::SetupInput(EventHandler& e)
	{
		e.BindAction(EventType::MouseScrolled, this, &Camera::OnMouseScroll);
		e.BindAction(EventType::ResizeWindow, this, &Camera::OnResizeWindow);
	}

	void Camera::OnMouseScroll(const MouseScrolledEvent& e)
	{
		m_Zoom -= e.GetYOffset() * 0.25f;
		m_Zoom = glm::max(m_Zoom, 0.25f);

		SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	}

	void Camera::OnResizeWindow(const ResizeWindowEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	}

	void Camera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}
