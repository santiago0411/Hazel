#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"

#include <entt.hpp>

class b2World;

namespace Hazel
{
	class Entity;
	using EntityMap = std::unordered_map<UUID, Entity>;

	struct TransformComponent;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		static Ref<Scene> Copy(const Ref<Scene>& scene);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUuid(UUID uuid, const std::string& name = std::string());
		void DuplicateEntity(Entity entity);
		void SubmitToDestroyEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateEditor(Timestep ts, const EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		Entity FindEntityByTag(const std::string& tag);
		Entity FindEntityByUuid(UUID id);

		void ConvertToLocalSpace(Entity entity);
		void ConvertToWorldSpace(Entity entity);
		glm::mat4 GetWorldSpaceTransformMatrix(Entity entity);
		TransformComponent GetWorldSpaceTransform(Entity entity);

		const EntityMap& GetEntityMap() const { return m_EntityIdMap; }
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		template <typename Fn>
		void SubmitPostUpdateFunc(Fn&& func)
		{
			m_PostUpdateQueue.emplace_back(func);
		}
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		EntityMap m_EntityIdMap;

		std::vector<std::function<void()>> m_PostUpdateQueue;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
