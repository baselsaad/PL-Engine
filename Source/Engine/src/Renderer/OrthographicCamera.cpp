#include "pch.h"
#include "OrthographicCamera.h"
#include "Event/Event.h"
#include "Event/Input.h"
#include "Event/EventHandler.h"
#include "Math/Math.h"
#include "glm/gtc/epsilon.hpp"
#include "Core/Engine.h"

namespace PAL
{
	OrthographicCamera::OrthographicCamera(float size, float nearClip /*= -1.0f*/, float farClip /*= 1.0f*/)
		: Camera(16.0f / 9.0f)
		, m_OrthographicSize(size)
		, m_OrthographicNear(nearClip)
		, m_OrthographicFar(farClip)
		, m_LastMousePos(0.0f)
	{
		float orthoWidth = m_OrthographicSize * m_AspectRatio;
		float orthoHeight = m_OrthographicSize;

		SetOrthoProjectionBounds(orthoWidth, orthoHeight, m_OrthographicNear, m_OrthographicFar);
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
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		}
		else if (Input::IsKeyPressed(KeyCode::S))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;
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

		m_Mvp = m_ProjectionMatrix * m_ViewTransformation;
	}

	void OrthographicCamera::OnMouseScroll(const MouseScrolledEvent& e)
	{
		m_OrthographicSize -= e.GetYOffset() * 0.25f;
		m_OrthographicSize = glm::max(m_OrthographicSize, 0.25f);

		float orthoWidth = m_OrthographicSize * m_AspectRatio;
		float orthoHeight = m_OrthographicSize;

		SetOrthoProjectionBounds(orthoWidth, orthoHeight, m_OrthographicNear, m_OrthographicFar);
	}

	void OrthographicCamera::OnResizeWindow(const ResizeWindowEvent& e)
	{
		SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
	}

	void OrthographicCamera::SetOrthoProjectionBounds(const float width, const float height, const float nearP, const float farP)
	{
		m_ProjectionMatrix = glm::ortho(-width , width , -height , height , nearP, farP);

		m_Mvp = m_ProjectionMatrix * m_ViewTransformation;
	}

	void OrthographicCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		float orthoWidth = m_OrthographicSize * m_AspectRatio;
		float orthoHeight = m_OrthographicSize;

		SetOrthoProjectionBounds(orthoWidth, orthoHeight, m_OrthographicNear, m_OrthographicFar);
	}

}