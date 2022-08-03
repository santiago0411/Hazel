#pragma once

#include <filesystem>

extern "C" {
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Hazel
{
	class Scene;
	class Entity;
	class UUID;
	class Timestep;

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);

		MonoObject* Instantiate() const;
		MonoMethod* GetMethod(const std::string& name, int parameterCount) const;
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** parameters = nullptr);

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(const Ref<ScriptClass>& scriptClass, UUID entityId);

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float ts) const;
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);

		static bool EntityClassExists(const std::string& fullClassName);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
		static MonoImage* GetCoreAssemblyImage();

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses(MonoAssembly* assembly);

		friend class ScriptClass;
		friend class ScriptRegistry;
	};
}
