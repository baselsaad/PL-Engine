#pragma once

namespace PAL
{
	class Camera;

	struct AABB
	{
		glm::vec3 Min;
		glm::vec3 Max;
	};

	class Ray {
	public:
		Ray(const glm::vec3& orgin, const glm::vec3& dir);
		bool PointInsideAABB(const glm::vec3& point, const AABB& aabb);

		inline const glm::vec3& GetOrigin() const { return m_Origin; }
		inline const glm::vec3& GetDirection() const { return m_Direction; }

	private:
		glm::vec3 m_Origin;
		glm::vec3 m_Direction;
	};
}
