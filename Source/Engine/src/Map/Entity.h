#pragma once
#include "ECS.h"
#include "World.h"

namespace PAL
{
	class Entity
	{
	public:
		Entity(World* world);
		virtual ~Entity();

		//TODO: AddComponent, GetComponent, HasComponent

		inline TransformComponent& GetTransform() { return m_World->GetRegisteredComponents().get<TransformComponent>(m_EntityID); }
		inline void SetColor(const RGBA& color) { m_World->GetRegisteredComponents().get<RenderComponent>(m_EntityID).Color = color; }
		inline void SetLayer(int64_t layer) { m_World->GetRegisteredComponents().get<LayerComponent>(m_EntityID).Layer = layer; }

		inline EntityID GetEntityID() const { return m_EntityID; }
		inline World* GetWorld() { return m_World; }

	private:
		EntityID m_EntityID;
		World* m_World;
	};

}