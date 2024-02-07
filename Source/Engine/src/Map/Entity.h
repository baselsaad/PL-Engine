#pragma once
#include "ECS.h"
#include "World.h"

namespace PAL
{
	class Entity
	{
	public:
		Entity();
		Entity(World* world);
		virtual ~Entity();

		//TODO: AddComponent, GetComponent, HasComponent

		template<typename T>
		T& GetComponent()
		{
			PAL_ASSERT(HasComponent<T>(), "Entity does not have component: {}", typeid(T).name());
			return m_World->m_RegisteredComponents.get<T>(m_EntityID);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_World->m_RegisteredComponents.any_of<T>(m_EntityID);
		}
		 
		template<typename T>
		void RemoveComponent()
		{
			PAL_ASSERT(HasComponent<T>(), "Entity does not have component: {}", typeid(T).name());
			m_World->m_RegisteredComponents.remove<T>(m_EntityID);
		}

		inline TransformComponent& GetTransform() { return m_World->GetRegisteredComponents().get<TransformComponent>(m_EntityID); }
		inline void SetColor(const RGBA& color) { m_World->GetRegisteredComponents().get<RenderComponent>(m_EntityID).Color = color; }
		inline void SetLayer(int64_t layer) { m_World->GetRegisteredComponents().get<LayerComponent>(m_EntityID).Layer = layer; }

		inline void SetEntityId(EntityID id) { m_EntityID = id; }
		inline EntityID GetEntityID() const { return m_EntityID; }

		inline void SetWorld(World* world) { m_World = world; }
		inline World* GetWorld() { return m_World; }

		operator bool() const { return m_EntityID != entt::null; }

		bool operator==(const Entity& other) const
		{
			return m_EntityID == other.m_EntityID && m_World == other.m_World;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	public:
		const static uint32_t INVALID_ENTITY_ID = std::numeric_limits<uint32_t>::max();

	private:
		EntityID m_EntityID;
		World* m_World;
	};

}