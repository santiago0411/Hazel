#include "hzpch.h"
#include "Hazel/Scene/Scene.h"

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Scene/Components.h"

#include "Hazel/Renderer/Renderer2D.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace Hazel
{
	using EntityId = entt::entity;

	static b2BodyType RigidBody2DTypeToBox2DBody(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:	return b2_staticBody;
			case RigidBody2DComponent::BodyType::Dynamic:	return b2_dynamicBody;
			case RigidBody2DComponent::BodyType::Kinematic:	return b2_kinematicBody;
		}

		HZ_CORE_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

	template<typename ... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, EntityId>& enttMap)
	{
		([&]()
			{
				src.view<Component>().each([&](EntityId srcEntity, Component& srcComponent)
				{
					EntityId dstEntity = enttMap.at(src.get<IdComponent>(srcEntity).Id);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				});
			}(), ...);
	}

	template<typename ... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, EntityId>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename ... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename ... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	static void CopyAllComponents(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, EntityId>& enttMap)
	{
		CopyComponent(AllComponents{}, dst, src, enttMap);
	}

	static void CopyAllExistingComponents(Entity dst, Entity src)
	{
		CopyComponentIfExists(AllComponents{}, dst, src);
	}

	Ref<Scene> Scene::Copy(const Ref<Scene>& scene)
	{
		auto newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = scene->m_ViewportWidth;
		newScene->m_ViewportHeight = scene->m_ViewportHeight;

		std::unordered_map<UUID, EntityId> enttMap;

		// Create entities in the new scene
		auto& srcSceneRegistry = scene->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		srcSceneRegistry.group<IdComponent, TagComponent>().each(
			[&](const IdComponent& idComponent, const TagComponent& tagComponent)
			{
				auto uuid = idComponent.Id;
				Entity newEntity = newScene->CreateEntityWithUuid(uuid, tagComponent.Tag);
				enttMap[uuid] = newEntity;
			});

		CopyAllComponents(dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUuid(UUID(), name);
	}

	Entity Scene::CreateEntityWithUuid(UUID uuid, const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<IdComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());
		CopyAllExistingComponents(newEntity, entity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		m_Registry.view<RigidBody2DComponent>().each([this](EntityId entityId, RigidBody2DComponent& rbc)
		{
			Entity entity = { entityId, this };
			auto& transform = entity.GetComponent<TransformComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = RigidBody2DTypeToBox2DBody(rbc.Type);
			bodyDef.position.Set(transform.Position.x, transform.Position.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rbc.FixedRotation);
			rbc.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = cc2d.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		});

		m_Registry.view<NativeScriptComponent>().each([this](EntityId entityId, NativeScriptComponent& nsc)
		{
			nsc.Instance = nsc.InstantiateScript();
			nsc.Instance->m_Entity = Entity{ entityId, this };
			nsc.Instance->OnCreate();
		});
	}

	void Scene::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateEditor(Timestep ts, const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		m_Registry.group<TransformComponent, SpriteRendererComponent>().each([](EntityId entityId, TransformComponent& tc, SpriteRendererComponent& src)
		{
			Renderer2D::DrawSprite(tc.GetTransform(), src, (int32_t)entityId);
		});

		m_Registry.view<TransformComponent, CircleRendererComponent>().each([](EntityId entityId, TransformComponent& tc, CircleRendererComponent& crc)
		{
			Renderer2D::DrawCircle(tc.GetTransform(), crc.Thickness, crc.Fade, crc.Color, (int32_t)entityId);
		});

		Renderer2D::EndScene();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// Update scripts
		m_Registry.view<NativeScriptComponent>().each([=, this](NativeScriptComponent& nsc)
		{
			nsc.Instance->OnUpdate(ts);
		});

		// Physics
		{
			constexpr int32_t velocityIterations = 6;
			constexpr int32_t positionIterations = 2;
			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

			m_Registry.view<RigidBody2DComponent>().each([this](EntityId entityId, RigidBody2DComponent& rbc)
			{
				Entity entity = { entityId, this };
				auto& transform = entity.GetComponent<TransformComponent>();

				const auto* body = static_cast<b2Body*>(rbc.RuntimeBody);
				const auto& position = body->GetPosition();
				transform.Position.x = position.x;
				transform.Position.y = position.y;
				transform.Rotation.z = body->GetAngle();
			});
		}

		// Render
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			const auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (const auto entity : view)
			{
				const auto [camera, transform] = view.get(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			m_Registry.group<TransformComponent, SpriteRendererComponent>().each([](EntityId entityId, TransformComponent& tc, SpriteRendererComponent& src)
			{
				Renderer2D::DrawSprite(tc.GetTransform(), src, (int32_t)entityId);
			});

			m_Registry.view<TransformComponent, CircleRendererComponent>().each([](EntityId entityId, TransformComponent& tc, CircleRendererComponent& crc)
			{
				Renderer2D::DrawCircle(tc.GetTransform(), crc.Thickness, crc.Fade, crc.Color, (int32_t)entityId);
			});

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_Registry.view<CameraComponent>().each([=](CameraComponent& cameraComponent)
		{
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		});
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		const auto view = m_Registry.view<CameraComponent>();
		for (const auto entityId : view)
		{
			const auto& camera = view.get<CameraComponent>(entityId);
			if (camera.Primary)
				return Entity{ entityId, this };
		}

		return {};
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<IdComponent>(Entity entity, IdComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{
	}
}
