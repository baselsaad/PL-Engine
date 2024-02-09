#pragma once

namespace PAL 
{
	struct QuadVertex
	{
		glm::vec3 Pos;
		glm::vec4 Color;
		glm::vec4 ObjectId;
	};

	class VertexBuffer
	{
	public:
		virtual void DestroyBuffer() = 0;
		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) = 0;
	};
}