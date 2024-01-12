#include "pch.h"
#include "OrthographicCamera.h"
#include "Event/Event.h"
#include "Event/Input.h"
#include "Event/EventHandler.h"
#include "Math/Math.h"
#include "glm/gtc/epsilon.hpp"

namespace PAL
{
	OrthographicCamera::OrthographicCamera(float aspectratio)
		: Camera(aspectratio)
		, m_Zoom(1.0f)
	{
		SetAspectRatio(aspectratio);
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
		m_ViewTransformation = glm::lookAt(m_CameraPosition, m_CameraPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_Mvp = m_ModelTransformation * m_ViewTransformation;
	}

	void OrthographicCamera::OnMouseScroll(const MouseScrolledEvent& e)
	{
		m_Zoom -= e.GetYOffset() * 0.25f;
		m_Zoom = glm::max(m_Zoom, 0.25f);

		SetOrthoProjectionBounds(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	}

	void OrthographicCamera::OnResizeWindow(const ResizeWindowEvent& e)
	{
		SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
	}

	void OrthographicCamera::SetAspectRatio(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		SetOrthoProjectionBounds(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	}

	void OrthographicCamera::SetOrthoProjectionBounds(float left, float right, float bottom, float top)
	{
		m_ModelTransformation = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_Mvp = m_ModelTransformation * m_ViewTransformation;
	}
}