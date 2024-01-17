#include "pch.h"
#include "HierarchyPanel.h"
#include "imgui.h"
#include "Map/Entity.h"


namespace PAL
{

	HierarchyPanel::HierarchyPanel(const SharedPtr<World>& world)
		: m_World(world)
	{
	}

	void HierarchyPanel::ImGuiRender()
	{
		ImGui::Begin("Hierarchy Panel");
		if (m_World)
		{
			auto view = m_World->GetRegisteredComponents().view<TransformComponent, RenderComponent>();
			for (auto [entityId, transform, renderComponent] : view.each())
			{
				Entity entity;
				entity.SetEntityId(entityId);
				entity.SetWorld(m_World.StdSharedPtr.get());

				DrawEntityHierarchy(entity);
			}

			if (ImGui::BeginPopupContextWindow(0, 1))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					Entity entity(m_World.StdSharedPtr.get());
					entity.GetComponent<TagComponent>().Tag = "New Entity";
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		ImGui::Begin("Properties");
		{
			// Components of selected entity 
		}
		ImGui::End();
	}

	void HierarchyPanel::DrawEntityHierarchy(Entity& entity)
	{
		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		auto& tagComponent = entity.GetComponent<TagComponent>();

		if (tagComponent.Tag.empty())
		{
			return;
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tagComponent.Tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tagComponent.Tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			//m_World->DestroyEntity(entity);
		}
	}
}