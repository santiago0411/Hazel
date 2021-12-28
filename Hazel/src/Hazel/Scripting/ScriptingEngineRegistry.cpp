#include "hzpch.h"
#include "ScriptingEngineRegistry.h"

#include "Hazel/Scripting/ScriptWrappers.h"

#include <mono/jit/jit.h>

#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel
{
	std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;

	extern MonoImage* s_EngineAssemblyImage;

#define Component_RegisterType(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("Hazel." #Type, s_EngineAssemblyImage);\
		if (type) {\
			uint32_t id = mono_type_get_type(type);\
			s_HasComponentFuncs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); };\
			s_CreateComponentFuncs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };\
		} else {\
			HZ_CORE_ERROR("No C# component class found for " #Type "!");\
		}\
	}

	static void InitComponentTypes()
	{
		Component_RegisterType(TagComponent);
		Component_RegisterType(TransformComponent);
		Component_RegisterType(ScriptComponent);
		Component_RegisterType(RigidBody2DComponent);
	}

	void ScriptingEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		// Input
		mono_add_internal_call("HazelEngine.Input::IsKeyPressed_Native", Scripting::Hazel_Input_IsKeyPressed);
		mono_add_internal_call("HazelEngine.Input::IsMouseButtonPressed_Native", Scripting::Hazel_Input_IsMouseButtonPressed);
		mono_add_internal_call("HazelEngine.Input::GetMousePosition_Native", Scripting::Hazel_Input_GetMousePosition);

		// Entity
		mono_add_internal_call("HazelEngine.Entity::CreateEntity_Native", Scripting::Hazel_Entity_CreateEntity);
		mono_add_internal_call("HazelEngine.Entity::Instantiate_Native", Scripting::Hazel_Entity_Instantiate);
		mono_add_internal_call("HazelEngine.Entity::InstantiateWithPosition_Native", Scripting::Hazel_Entity_Entity_InstantiateWithPosition);
		mono_add_internal_call("HazelEngine.Entity::InstantiateWithTransform_Native", Scripting::Hazel_Entity_Entity_InstantiateWithTransform);
		mono_add_internal_call("HazelEngine.Entity::DestroyEntity_Native", Scripting::Hazel_Entity_DestroyEntity);
		mono_add_internal_call("HazelEngine.Entity::CreateComponent_Native", Scripting::Hazel_Entity_CreateComponent);
		mono_add_internal_call("HazelEngine.Entity::HasComponent_Native", Scripting::Hazel_Entity_HasComponent);
		mono_add_internal_call("HazelEngine.Entity::FindEntityByTag_Native", Scripting::Hazel_Entity_FindEntityByTag);

		// TagComponent
		mono_add_internal_call("HazelEngine.TagComponent::GetTag_Native", Scripting::Hazel_TagComponent_GetTag);
		mono_add_internal_call("HazelEngine.TagComponent::SetTag_Native", Scripting::Hazel_TagComponent_SetTag);

		// TransformComponent
		mono_add_internal_call("HazelEngine.TransformComponent::GetTransform_Native", Scripting::Hazel_TransformComponent_GetTransform);
		mono_add_internal_call("HazelEngine.TransformComponent::SetTransform_Native", Scripting::Hazel_TransformComponent_SetTransform);
		mono_add_internal_call("HazelEngine.TransformComponent::GetPosition_Native", Scripting::Hazel_TransformComponent_GetPosition);
		mono_add_internal_call("HazelEngine.TransformComponent::SetPosition_Native", Scripting::Hazel_TransformComponent_SetPosition);
		mono_add_internal_call("HazelEngine.TransformComponent::GetRotation_Native", Scripting::Hazel_TransformComponent_GetRotation);
		mono_add_internal_call("HazelEngine.TransformComponent::SetRotation_Native", Scripting::Hazel_TransformComponent_SetRotation);
		mono_add_internal_call("HazelEngine.TransformComponent::GetScale_Native", Scripting::Hazel_TransformComponent_GetScale);
		mono_add_internal_call("HazelEngine.TransformComponent::SetScale_Native", Scripting::Hazel_TransformComponent_SetScale);
		mono_add_internal_call("HazelEngine.TransformComponent::GetWorldSpaceTransform_Native", Scripting::Hazel_TransformComponent_GetWorldSpaceTransform);

		// ScriptComponent
		mono_add_internal_call("HazelEngine.ScriptComponent::GetInstance_Native", Scripting::Hazel_ScriptComponent_GetInstance);
	}
}
