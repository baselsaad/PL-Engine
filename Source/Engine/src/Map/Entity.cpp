#include "pch.h"
#include "Entity.h"


namespace PAL
{
	Entity::Entity(World* world)
		: m_World(world)
	{
		PAL_ASSERT(world, "World is null!!!");
		//@TODO: Prefab & GetId only when begin play
		m_EntityID = m_World->OnCreateNewEntity();

		m_World->RegisterComponent<TransformComponent>(this);
		m_World->RegisterComponent<RenderComponent>(this);
		m_World->RegisterComponent<TagComponent>(this);
	}

	Entity::Entity()
	{
		m_EntityID = entt::null;
	}

	Entity::~Entity()
	{

	}

}