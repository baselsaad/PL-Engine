#pragma once

namespace PAL
{
	using Registry = entt::registry;
	using EntityID = entt::entity;

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{
		}

		glm::mat4 GetTransformMatrix() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct RenderComponent
	{
		// Texture id
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f ,1.0f };

		RenderComponent() = default;
		RenderComponent(const RenderComponent&) = default;
		RenderComponent(const glm::vec4& color)
			: Color(color)
		{
		}

		glm::vec4& GetColor() { return Color; }
	};

	struct LayerComponent
	{
		int64_t Layer;

		inline void SetLayer(int64_t layer) { Layer = layer; }
		inline int64_t GetLayer() { return Layer; }
	};

	struct TagComponent
	{
		std::string Tag;

		inline void SetLayer(std::string_view tag) { Tag = tag; }
		inline std::string_view GetTag() { return Tag; }
	};

}