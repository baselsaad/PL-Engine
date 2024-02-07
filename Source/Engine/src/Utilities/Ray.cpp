#include "pch.h"
#include "Ray.h"
#include "Renderer/Camera.h"

namespace PAL
{
	Ray::Ray(const glm::vec3& orgin, const glm::vec3& dir)
		: m_Origin(orgin)
		, m_Direction(dir)
	{
	}

	
}