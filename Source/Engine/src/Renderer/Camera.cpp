#include "pch.h"
#include "Camera.h"
#include "Core/Engine.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Event/Event.h"
#include "Event/EventHandler.h"
#include "Event/Input.h"

namespace PAL
{
	// Res: https://gamedev.stackexchange.com/questions/191355/convert-world-to-screen-coordinates-with-camera-position-and-camera-angle

	glm::vec3 Camera::ScreenPosToWorld(glm::vec2 screenPos, glm::vec2 viewportSize)
	{
		//float s[3];
		//s[0] = 2 * screen[0] / viewportWidth - 1;
		//s[1] = 2 * screen[1] / viewportHeight - 1;
		//s[2] = screen[2];

		//float p[4];
		//p[0] = (s[0] * MVPinv[0][0]) + (s[1] * MVPinv[1][0]) + (s[2] * MVPinv[2][0]) + MVPinv[3][0];
		//p[1] = (s[0] * MVPinv[0][1]) + (s[1] * MVPinv[1][1]) + (s[2] * MVPinv[2][1]) + MVPinv[3][1];
		//p[2] = (s[0] * MVPinv[0][2]) + (s[1] * MVPinv[1][2]) + (s[2] * MVPinv[2][2]) + MVPinv[3][2];
		//p[3] = (s[0] * MVPinv[0][3]) + (s[1] * MVPinv[1][3]) + (s[2] * MVPinv[2][3]) + MVPinv[3][3];

		//float w = 1 / p[3];
		//world[0] = p[0] * w;
		//world[1] = p[1] * w;
		//world[2] = p[2] * w;

		float x = 2 * screenPos.x / viewportSize.x - 1;
		float y = 1 - 2 * screenPos.y / viewportSize.y; // No need to flip y-coordinate for Vulkan
		float z = 0.0f;

		glm::mat4 MVPinv = glm::inverse(GetModelViewProjection());
		glm::vec4 pixel = glm::vec4(x, y, z, 1.0f); 

		// Convert screen coordinates to world coordinates
		glm::vec4 worldPosition = MVPinv * pixel;
		worldPosition /= worldPosition.w; // Convert from homogeneous coordinates to Cartesian coordinates
		
		return worldPosition;
	}

	//void WorldToScreen(float screen[3], float world[3])
	//{
	//	float s[4];
	//	s[0] = (world[0] * MVP[0][0]) + (world[1] * MVP[1][0]) + (world[2] * MVP[2][0]) + MVP[3][0];
	//	s[1] = (world[0] * MVP[0][1]) + (world[1] * MVP[1][1]) + (world[2] * MVP[2][1]) + MVP[3][1];
	//	s[2] = (world[0] * MVP[0][2]) + (world[1] * MVP[1][2]) + (world[2] * MVP[2][2]) + MVP[3][2];
	//	s[3] = (world[0] * MVP[0][3]) + (world[1] * MVP[1][3]) + (world[2] * MVP[2][3]) + MVP[3][3];

	//	screen[0] = s[0] / s[3] * viewportWidth / 2 + viewportWidth / 2;
	//	screen[1] = s[1] / s[3] * viewportHeight / 2 + viewportHeight / 2;
	//	screen[2] = s[2] / s[3];
	//}

}
