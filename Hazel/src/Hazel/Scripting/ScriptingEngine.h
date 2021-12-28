#pragma once

#include "Hazel/Core/UUID.h"

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Scene.h"

extern "C" {
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClass MonoClass;
}

namespace Hazel
{
	struct EntityScriptClass;
	struct EntityInstance
	{
		EntityScriptClass* ScriptClass = nullptr;

		uint32_t Handle = 0;
		Scene* SceneInstance = nullptr;

		MonoObject* GetInstance() const;
		bool IsRuntimeAvailable() const;
	};

	struct EntityInstanceData
	{
		EntityInstance Instance;
		ScriptModuleFieldMap ModuleFieldMap;
	};

	using EntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, EntityInstanceData>>;

	class ScriptingEngine
	{
	public:
		static void Init(const char* engineAssemblyPath);
		static void Shutdown();

		static void OnSceneDestruct(UUID sceneId);

		static void SetSceneContext(const Ref<Scene>& scene);
		static Ref<Scene>& GetCurrentSceneContext();


		static void RunStaticMethod(const char* methodName);
		static void Test();

	private:
		static void RegisterMethods();
	};
}
