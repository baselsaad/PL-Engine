#include "pch.h"
#include "World.h"
#include "Renderer/Renderer.h"
#include "Utilities/Colors.h"
#include "Entity.h"
#include "Renderer/OrthographicCamera.h"
#include "Core/Engine.h"
#include "ECS.h"


namespace PAL
{
	static void BenchmarkBatchRenderer(World* world, OrthographicCamera& camera)
	{
		//float cord = camera.GetZoom() * 2.0f;
		float cord = 10.0f;
		const glm::vec3 scale(0.45f);

		for (float y = -cord; y < 5.0f; y += 0.5f)
		{
			for (float x = -cord; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };

				Entity entityTest(world);
				entityTest.GetTransform().Translation = { x, y, 0.0f };
				entityTest.GetTransform().Scale = glm::vec3(0.45f);
				entityTest.SetColor(color);
			}
		}
	}

	World::World()
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
		Entity entityTest(this);
		entityTest.SetColor(Colors::Orange);

		Entity entityTest2(this);
		entityTest2.SetColor(Colors::Blue);
		entityTest2.GetTransform().Translation.x = 2.0f;

		if (m_ActiveCamera == nullptr)
		{
			m_ActiveCamera = MakeShared<OrthographicCamera>(Engine::Get()->GetWindow()->GetAspectRatio());
			m_ActiveCamera->SetupInput(Engine::Get()->GetInputHandler());
		}

		BenchmarkBatchRenderer(this, (OrthographicCamera&)*m_ActiveCamera);
	}

	void World::OnUpdate(float deltaTime)
	{
		// Update Script and physics later		
	}

	void World::OnRender(float deltaTime, const SharedPtr<Renderer>& renderer)
	{
		m_ActiveCamera->OnUpdate(deltaTime);

		renderer->StartFrame(*m_ActiveCamera);

		auto view = m_RegisteredComponents.view<TransformComponent, RenderComponent>();
		for (auto [entity, transform, renderComponent] : view.each())
		{
			renderer->DrawQuad(transform, renderComponent.GetColor());
		}

		renderer->EndFrame();
	}

	void World::EndPlay()
	{

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

}