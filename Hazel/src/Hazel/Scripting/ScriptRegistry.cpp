#include "hzpch.h"
#include "ScriptRegistry.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>


#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name);

namespace Hazel
{
	static Entity GetEntity(UUID entityId)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		return scene->GetEntityByUUID(entityId);
	}

	static void NativeLog(MonoString* string, int parameter)
	{
		char* str = mono_string_to_utf8(string);
		HZ_CORE_INFO("{0}, {1}", str, parameter);
		mono_free(str);
	}

	static void NativeLog_Vector(glm::vec3* parameter)
	{
		HZ_CORE_INFO("Value: vec3({0}, {1}, {2})", parameter->x, parameter->y, parameter->z);
	}

	static void Entity_GetPosition(uint64_t entityId, glm::vec3* outPosition)
	{
		*outPosition = GetEntity(entityId).GetComponent<TransformComponent>().Position;
	}

	static void Entity_SetPosition(uint64_t entityId, glm::vec3* position)
	{
		GetEntity(entityId).GetComponent<TransformComponent>().Position = *position;
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	void ScriptRegistry::RegisterMethods()
	{
		HZ_ADD_INTERNAL_CALL(NativeLog);
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector);

		HZ_ADD_INTERNAL_CALL(Entity_GetPosition);
		HZ_ADD_INTERNAL_CALL(Entity_SetPosition);

		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
