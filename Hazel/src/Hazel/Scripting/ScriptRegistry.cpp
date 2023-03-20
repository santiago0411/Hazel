#include "hzpch.h"
#include "ScriptRegistry.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Physics/Physics2D.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>	

#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name);

namespace Hazel
{
	namespace Utils
	{
		std::string MonoStringToString(MonoString* string)
		{
			char* cStr = mono_string_to_utf8(string);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}
	}

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

	static void RigidBody2DComponent_GetLinearVelocity(uint64_t entityId, glm::vec2* outLinearVelocity)
	{
		Entity entity = GetEntity(entityId);
		const auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		auto* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& lv = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(lv.x, lv.y);
	}

	static RigidBody2DComponent::BodyType RigidBody2DComponent_GetType(uint64_t entityId)
	{
		Entity entity = GetEntity(entityId);
		const auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		auto* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Box2DBodyToRigidBody2DType(body->GetType());
	}

	static void RigidBody2DComponent_SetType(uint64_t entityId, RigidBody2DComponent::BodyType type)
	{
		Entity entity = GetEntity(entityId);
		const auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		auto* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::RigidBody2DTypeToBox2DBody(type));
	}

	static void SpriteRendererComponent_GetColor(uint64_t entityId, glm::vec4* outColor)
	{
		*outColor = GetEntity(entityId).GetComponent<SpriteRendererComponent>().Color;
	}

	static void SpriteRendererComponent_SetColor(uint64_t entityId, glm::vec4* color)
	{
		GetEntity(entityId).GetComponent<SpriteRendererComponent>().Color = *color;
	}

	static MonoString* TextComponent_GetText(uint64_t entityId)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		auto& str = entity.GetComponent<TextComponent>().TextString;
		return ScriptEngine::CreateString(str.c_str());
	}

	static void TextComponent_SetText(uint64_t entityId, MonoString* monoString)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		entity.GetComponent<TextComponent>().TextString = Utils::MonoStringToString(monoString);
	}

	static float TextComponent_GetKerning(uint64_t entityId)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		return entity.GetComponent<TextComponent>().Kerning;
	}

	static void TextComponent_SetKerning(uint64_t entityId, float kerning)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		entity.GetComponent<TextComponent>().Kerning = kerning;
	}

	static float TextComponent_GetLineSpacing(uint64_t entityId)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		return entity.GetComponent<TextComponent>().LineSpacing;
	}

	static void TextComponent_SetLineSpacing(uint64_t entityId, float lineSpacing)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		entity.GetComponent<TextComponent>().LineSpacing = lineSpacing;
	}

	static void TextComponent_GetColor(uint64_t entityId, glm::vec4* outColor)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		*outColor = entity.GetComponent<TextComponent>().Color;
	}

	static void TextComponent_SetColor(uint64_t entityId, glm::vec4* color)
	{
		Entity entity = GetEntity(entityId);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>())
		entity.GetComponent<TextComponent>().Color = *color;
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
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
		s_EntityHasComponentFunctions.clear();
		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::RegisterMethods()
	{
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent)
		HZ_ADD_INTERNAL_CALL(Entity_FindEntityByName)
		HZ_ADD_INTERNAL_CALL(GetScriptInstance)

		HZ_ADD_INTERNAL_CALL(TransformComponent_GetPosition)
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetPosition)
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation)
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation)
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetScale)
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetScale)

		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse)
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter)
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_GetLinearVelocity)
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_GetType)
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_SetType)

		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor)
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor)

		HZ_ADD_INTERNAL_CALL(TextComponent_GetText)
		HZ_ADD_INTERNAL_CALL(TextComponent_SetText)
		HZ_ADD_INTERNAL_CALL(TextComponent_GetKerning)
		HZ_ADD_INTERNAL_CALL(TextComponent_SetKerning)
		HZ_ADD_INTERNAL_CALL(TextComponent_GetLineSpacing)
		HZ_ADD_INTERNAL_CALL(TextComponent_SetLineSpacing)
		HZ_ADD_INTERNAL_CALL(TextComponent_GetColor)
		HZ_ADD_INTERNAL_CALL(TextComponent_SetColor)

		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown)
	}
}
