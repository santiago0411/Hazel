#include "hzpch.h"
#include "ScriptRegistry.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>	

#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name);

namespace Hazel
{ 
	static std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_EntityHasComponentFunctions;

	static Entity GetEntity(UUID entityId)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity);
		return entity;
	}

	static bool Entity_HasComponent(uint64_t entityId, MonoReflectionType* componentType)
	{
		Entity entity = GetEntity(entityId);
		MonoType* managedType = mono_reflection_type_get_type(componentType);

		HZ_CORE_ASSERT(s_EntityHasComponentFunctions.find(managedType) != s_EntityHasComponentFunctions.end());
		return s_EntityHasComponentFunctions.at(managedType)(entity);
	}

	static uint64_t Entity_FindEntityByName(MonoString* monoString)
	{
		char* cStr = mono_string_to_utf8(monoString);

		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(cStr);
		mono_free(cStr);

		return entity ? entity.GetUUID() : 0;
	}

	static MonoObject* GetScriptInstance(uint64_t entityId)
	{
		return ScriptEngine::GetManagedInstance(entityId);
	}

	static void TransformComponent_GetPosition(uint64_t entityId, glm::vec3* outPosition)
	{
		*outPosition = GetEntity(entityId).GetComponent<TransformComponent>().Position;
	}

	static void TransformComponent_SetPosition(uint64_t entityId, glm::vec3* position)
	{
		GetEntity(entityId).GetComponent<TransformComponent>().Position = *position;
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	static void TransformComponent_GetRotation(uint64_t entityId, glm::vec3* outRotation)
	{
		*outRotation = GetEntity(entityId).GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetRotation(uint64_t entityId, glm::vec3* rotation)
	{
		GetEntity(entityId).GetComponent<TransformComponent>().Rotation = *rotation;
	}

	static void TransformComponent_GetScale(uint64_t entityId, glm::vec3* outScale)
	{
		*outScale = GetEntity(entityId).GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(uint64_t entityId, glm::vec3* scale)
	{
		GetEntity(entityId).GetComponent<TransformComponent>().Scale = *scale;
	}

	static void RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityId, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Entity entity = GetEntity(entityId);
		const auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		auto* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void RigidBody2DComponent_ApplyLinearImpulseToCenter(uint64_t entityId, glm::vec2* impulse, bool wake)
	{
		Entity entity = GetEntity(entityId);
		const auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		auto* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForceToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void SpriteRendererComponent_GetColor(uint64_t entityId, glm::vec4* outColor)
	{
		*outColor = GetEntity(entityId).GetComponent<SpriteRendererComponent>().Color;
	}

	static void SpriteRendererComponent_SetColor(uint64_t entityId, glm::vec4* color)
	{
		GetEntity(entityId).GetComponent<SpriteRendererComponent>().Color = *color;
	}

	template<typename ... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				HZ_CORE_ASSERT(pos != std::string_view::npos, "All components must exist under Hazel namespace.");

				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypeName = fmt::format("Hazel.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					HZ_CORE_ERROR("ScriptRegistry::RegisterComponent could not find component type {}", managedTypeName);
					return;
				}

				s_EntityHasComponentFunctions[managedType] = [](Entity& entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptRegistry::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::RegisterMethods()
	{
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		HZ_ADD_INTERNAL_CALL(GetScriptInstance);

		HZ_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);

		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);

		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
