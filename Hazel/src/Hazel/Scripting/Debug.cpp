#include "hzpch.h"
#include "Debug.h"

#include <Hazel/Core/Log.h>

namespace Hazel::Scripting
{
	void Debug::RegisterMethods()
	{
		mono_add_internal_call("HazelEngine.Debug::LogDebug_Native", LogDebug_Native);
		mono_add_internal_call("HazelEngine.Debug::LogInfo_Native", LogInfo_Native);
		mono_add_internal_call("HazelEngine.Debug::LogWarn_Native", LogWarn_Native);
		mono_add_internal_call("HazelEngine.Debug::LogError_Native", LogError_Native);
	}

	void Debug::LogDebug_Native(MonoString* message)
	{
		HZ_SCRIPT_TRACE(mono_string_to_utf8(message));
	}

	void Debug::LogInfo_Native(MonoString* message)
	{
		HZ_SCRIPT_INFO(mono_string_to_utf8(message));
	}

	void Debug::LogWarn_Native(MonoString* message)
	{
		HZ_SCRIPT_WARN(mono_string_to_utf8(message));
	}

	void Debug::LogError_Native(MonoString* message)
	{
		HZ_SCRIPT_ERROR(mono_string_to_utf8(message));
	}
}
