#pragma once
#include <entt/entt.hpp>
#include "ECS.h"

namespace PAL
{
	class Camera;
	class Entity;
	class EventHandler;
	class RuntimeRenderer;

	class World
	{
	public:
		World();
		virtual ~World();

		void BeginPlay();
		void SetupInput(EventHandler& eventHandler);
		void OnUpdate(float deltaTime);
		void OnRender(float deltaTime);
		void EndPlay();

		

		template<typename ComponentType>
		void RegisterComponent(Entity* entity);

		//TODO: remove later
		// Create New Entity
		EntityID OnCreateNewEntity();

		inline Registry& GetRegisteredComponents() { return m_RegisteredComponents; }
		inline const std::string& GetWorldName() const { return m_WorldName; }
		inline const SharedPtr<Camera>& GetActiveCamera() const { return m_ActiveCamera; }

	private:
		template<typename ComponentType>
		ComponentType& RegisterComponentInternal(EntityID entityId)
		{
			// do not store a ref to the component, it will be invalid while resizing the pool when adding new components 
			ComponentType& newComponent = m_RegisteredComponents.emplace<ComponentType>(entityId);
			return newComponent;
		}

	private:
		Registry m_RegisteredComponents;
		SharedPtr<Camera> m_ActiveCamera;
		std::string m_WorldName;

		friend class Entity;
	};

	


}

