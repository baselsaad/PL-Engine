#include "pch.h"
#include "World.h"

#include "Renderer/RuntimeRenderer.h"
#include "Renderer/OrthographicCamera.h"

#include "Core/Engine.h"
#include "Event/EventHandler.h"
#include "Utilities/Colors.h"
#include "Entity.h"
#include "ECS.h"
#include "Utilities/Timer.h"

namespace PAL
{
	static void BenchmarkBatchRenderer(World* world, OrthographicCamera& camera)
	{
		//float cord = 160.0f; // 100.000 Quads
		float cord = 5.0f;
		const glm::vec3 scale(0.45f);

		for (float y = -cord; y < 5.0f; y += 0.5f)
		{
			for (float x = -cord; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };

				Entity entityTest(world);
				entityTest.GetTransform().Translation = { x, y, -1.0f };
				entityTest.GetTransform().Scale = glm::vec3(0.45f);
				entityTest.SetColor(color);
			}
		}
	}

	World::World()
		: m_WorldName("DefaultWorld")
	{
	}

	World::~World()
	{
	}

	EntityID World::OnCreateNewEntity()
	{
		return m_RegisteredComponents.create();
	}

	void World::BeginPlay()
	{
		CORE_PROFILER_FUNC();

		Entity entityTest(this);
		entityTest.SetColor(Colors::Dark_Orange);
		entityTest.GetComponent<TagComponent>().Tag = "OrangeEntity";
		
		Entity entityTest2(this);
		entityTest2.SetColor(Colors::Blue);
		entityTest2.GetTransform().Translation.x = 2.0f;
		entityTest2.GetComponent<TagComponent>().Tag = "BlueEntity";

		BenchmarkBatchRenderer(this, (OrthographicCamera&)*m_ActiveCamera);

		if (m_ActiveCamera == nullptr)
		{
			m_ActiveCamera = NewShared<OrthographicCamera>(5.0f, -1.0f, 1.0f);
		}
	}

	void World::SetupInput(EventHandler& eventHandler)
	{
		m_ActiveCamera->SetupInput(eventHandler);
	}

	void World::OnUpdate(float deltaTime)
	{
		CORE_PROFILER_FUNC();

		// Update Script and physics later		
	}

	void World::OnRender(float deltaTime)
	{
		CORE_PROFILER_FUNC();

		m_ActiveCamera->OnUpdate(deltaTime);
		Engine::Get()->GetRuntimeRenderer()->SetProjection(m_ActiveCamera->GetModelViewProjection());

		auto view = m_RegisteredComponents.view<TransformComponent, RenderComponent>();
		for (auto [entity, transform, renderComponent] : view.each())
		{
			Engine::Get()->GetRuntimeRenderer()->DrawQuad(transform, renderComponent.GetColor(), (uint32_t)entity + 100);
		}
	}

	void World::EndPlay()
	{
		CORE_PROFILER_FUNC();
	}

	template<typename ComponentType>
	void World::RegisterComponent(Entity* entity)
	{
		static_assert(false, "Unknown ComponentType");
	}

	template<>
	void World::RegisterComponent<TransformComponent>(Entity* entity)
	{
		RegisterComponentInternal<TransformComponent>(entity->GetEntityID());
	}

	template<>
	void World::RegisterComponent<RenderComponent>(Entity* entity)
	{
		RegisterComponentInternal<RenderComponent>(entity->GetEntityID());
	}

	template<>
	void World::RegisterComponent<LayerComponent>(Entity* entity)
	{
		RegisterComponentInternal<LayerComponent>(entity->GetEntityID());
	}

	template<>
	void World::RegisterComponent<TagComponent>(Entity* entity)
	{
		RegisterComponentInternal<TagComponent>(entity->GetEntityID());
	}

}