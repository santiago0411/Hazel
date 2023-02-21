#include "hzpch.h"
#include "Hazel/Scene/Scene.h"

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Scene/Components.h"

#include "Hazel/Scripting/ScriptEngine.h"

#include "Hazel/Renderer/Renderer2D.h"

#include "Hazel/Physics/Physics2D.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace Hazel
{
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

	Scene::Scene()
	{
		m_Registry.on_construct<CameraComponent>().connect<&Scene::OnCameraComponentAdded>(this);
		m_Registry.on_construct<NativeScriptComponent>().connect<&Scene::OnNativeScriptComponentAdded>(this);
	}

	Scene::~Scene()
	{
		m_Registry.on_destroy<CameraComponent>().disconnect();
		m_Registry.on_destroy<NativeScriptComponent>().disconnect();
		delete m_PhysicsWorld;
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

		srcSceneRegistry.group<IdComponent, TagComponent>().each([&](const IdComponent& idComponent, const TagComponent& tagComponent)
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
		m_EntityMap[uuid] = entity;
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());
		CopyAllExistingComponents(newEntity, entity);
		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;
		InitPhysics2D();

		// Scripting
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;
		StopPhysics2D();
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
		InitPhysics2D();
	}

	void Scene::OnSimulationStop()
	{
		StopPhysics2D();
	}

	void Scene::OnUpdateEditor(Timestep ts, const EditorCamera& camera)
	{
		RenderScene(camera);
	}

	void Scene::OnUpdateSimulation(Timestep ts, const EditorCamera& camera)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
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
		}

		RenderScene(camera);
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			// Update scripts
			{
				auto view = m_Registry.view<ScriptComponent>();
				for (auto e : view)
				{
					Entity entity{ e, this };
					ScriptEngine::OnUpdateEntity(entity, ts);
				}

				m_Registry.view<NativeScriptComponent>().each([=](NativeScriptComponent& nsc)
				{
					nsc.Instance->OnUpdate(ts);
				});
			}

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
				Renderer2D::DrawCircle(tc.GetTransform(), crc.Color, crc.Thickness, crc.Fade, (int32_t)entityId);
			});

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_Registry.view<CameraComponent>().each([=](CameraComponent& cameraComponent)
		{
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		});
	}

	Entity Scene::GetEntityByUUID(UUID id)
	{
		HZ_CORE_ASSERT(m_EntityMap.find(id) != m_EntityMap.end());
		return { m_EntityMap.at(id), this };
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		const auto view = m_Registry.view<TagComponent>();
		for (const auto entityId : view)
		{
			if (view.get<TagComponent>(entityId).Tag == name)
				return { entityId, this };
		}

		return {};
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

	void Scene::InitPhysics2D()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		m_Registry.view<RigidBody2DComponent>().each([this](EntityId entityId, RigidBody2DComponent& rbc)
		{
			Entity entity = { entityId, this };
			auto& transform = entity.GetComponent<TransformComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Utils::RigidBody2DTypeToBox2DBody(rbc.Type);
			bodyDef.position.Set(transform.Position.x, transform.Position.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rbc.FixedRotation);
			rbc.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y,
					b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

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
				circleShape.m_radius = transform.Scale.x * cc2d.Radius;

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

	void Scene::StopPhysics2D()
	{
		m_Registry.view<NativeScriptComponent>().each([](EntityId entityId, NativeScriptComponent& nsc)
		{
			nsc.Instance->OnDestroy();
		});

		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::RenderScene(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		m_Registry.group<TransformComponent, SpriteRendererComponent>().each([](EntityId entityId, TransformComponent& tc, SpriteRendererComponent& src)
		{
			Renderer2D::DrawSprite(tc.GetTransform(), src, (int32_t)entityId);
		});

		m_Registry.view<TransformComponent, CircleRendererComponent>().each([](EntityId entityId, TransformComponent& tc, CircleRendererComponent& crc)
		{
			Renderer2D::DrawCircle(tc.GetTransform(), crc.Color, crc.Thickness, crc.Fade, (int32_t)entityId);
		});

		Renderer2D::DrawString("Santi", Font::GetDefault(), glm::mat4(1.0f), glm::vec4(1.0f));
		Renderer2D::DrawString(
		R"(layout(location = 0) out vec4 o_Color;
		layout(location = 1) out int o_EntityId;

		struct VertexOutput
		{
			vec4 Color;
			vec2 TexCoord;
		};

		layout(location = 0) in VertexOutput Input;
		layout(location = 2) in flat int v_EntityId;

		layout(binding = 0) uniform sampler2D u_FontAtlas;

		float screenPxRange()
		{
			const float pxRange = 2.0;
			vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlas, 0));
			vec2 screenTexSize = vec2(1.0) / fwidth(Input.TexCoord);
			return max(0.5 * dot(unitRange, screenTexSize), 1.0);
		}

		float median(float r, float g, float b)
		{
			return max(min(r, g), min(max(r, g), b));
		}

		void main()
		{
			vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
			float sd = median(msd.r, msd.g, msd.b);
			float screenPxDistance = screenPxRange() * (sd - 0.5);
			float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

			if (opacity == 0.0)
				discard;

			vec4 bgColor = vec4(0.0);
			o_Color = mix(bgColor, Input.Color, opacity);

			if (o_Color.a == 0.0)
				discard;

			o_EntityId = v_EntityId;
		})"
		, Font::GetDefault(), glm::mat4(1.0f), glm::vec4(1.0f));

		Renderer2D::EndScene();
	}

	void Scene::OnCameraComponentAdded(entt::registry& registry, entt::entity entity) const
	{
		auto& component = registry.get<CameraComponent>(entity);
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	void Scene::OnNativeScriptComponentAdded(entt::registry& registry, entt::entity entity)
	{
		if (!m_IsRunning)
			return;

		auto& nsc = registry.get<NativeScriptComponent>(entity);
		nsc.Instance = nsc.InstantiateScript();
		nsc.Instance->m_Entity = Entity{ entity, this };
		nsc.Instance->OnCreate();
	}
}
