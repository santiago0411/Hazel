#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"

#include <entt.hpp>

class b2World;

namespace Hazel
{
	class Entity;
	using EntityId = entt::entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(const Ref<Scene>& scene);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUuid(UUID uuid, const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera);
		void OnUpdateSimulation(Timestep ts, const EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetEntityByUUID(UUID id);
		Entity FindEntityByName(std::string_view name);

		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void SetPaused(bool paused) { m_IsPaused = paused; }

		void Step(int frames = 1) { m_StepFrames = frames; }

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void InitPhysics2D();
		void StopPhysics2D();

		void RenderScene(const EditorCamera& camera);

		void OnCameraComponentAdded(entt::registry& registry, entt::entity entity) const;
		void OnNativeScriptComponentAdded(entt::registry& registry, entt::entity entity);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		b2World* m_PhysicsWorld = nullptr;

		std::unordered_map<UUID, EntityId> m_EntityMap;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
