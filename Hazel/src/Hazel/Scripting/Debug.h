#pragma once

#include <mono/metadata/object.h>

namespace Hazel::Scripting
{
	class Debug
	{
	public:
		static void RegisterMethods();

	private:
		static void LogDebug_Native(MonoString* message);
		static void LogInfo_Native(MonoString* message);
		static void LogWarn_Native(MonoString* message);
		static void LogError_Native(MonoString* message);
	};
}
