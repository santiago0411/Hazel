#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"

#include <entt.hpp>

class b2World;

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene();

		static Ref<Scene> Copy(const Ref<Scene>& scene);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUuid(UUID uuid, const std::string& name = std::string());
		void DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera);
		void OnUpdateSimulation(Timestep ts, const EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();

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
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
