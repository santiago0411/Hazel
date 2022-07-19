#include "hzpch.h"
#include "ScriptRegistry.h"

#include <mono/metadata/object.h>

namespace Hazel
{
#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name);

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

	void ScriptRegistry::RegisterMethods()
	{
		HZ_ADD_INTERNAL_CALL(NativeLog);
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector);
	}
}
