#include "hzpch.h"
#include "Hazel/Scene/Scene.h"

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"

#include "Hazel/Renderer/Renderer2D.h"

namespace Hazel
{
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		const auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (const auto entity : group)
		{
			const auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}
}
