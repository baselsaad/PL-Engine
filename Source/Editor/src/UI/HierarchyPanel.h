#pragma once
#include "Map/Entity.h"


namespace PAL
{
	class HierarchyPanel
	{
	public:
		HierarchyPanel(const SharedPtr<World>& world);

		void ImGuiRender();
		void DrawEntityHierarchy(Entity& entity, int openAction);
		void DrawComponents(Entity& entity);

		inline void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }
		inline void SetWorld(const SharedPtr<World>& world) { m_World = world; }

	private:
		SharedPtr<World> m_World;
		Entity m_SelectedEntity;
	};
}