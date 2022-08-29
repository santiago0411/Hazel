#pragma once

#include <filesystem>
#include <map>

extern "C" {
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoClassField MonoClassField;
}

namespace Hazel
{
	enum class ScriptFieldType
	{
		None = 0,
		Boolean, Byte, SByte, UShort, Short, UInt, Int, ULong, Long,
		Float, Double, Decimal,
		Char, String,
		Vector2, Vector3, Vector4, Color,
		Entity
	};

	struct ScriptField
	{
		std::string Name;
		ScriptFieldType Type;
		MonoClassField* Field;
	};

	class Scene;
	class Entity;
	class UUID;
	class Timestep;

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);
		ScriptClass(MonoClass* cls);

		MonoObject* Instantiate() const;
		MonoMethod* GetMethod(const std::string& name, int parameterCount) const;
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** parameters = nullptr);

		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

		friend class ScriptEngine;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(const Ref<ScriptClass>& scriptClass, UUID entityId);

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float ts) const;

		Ref<ScriptClass> GetScriptClass() const { return m_ScriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			bool success = GetFieldValueInternal(name);
			return success ? *(T*)s_FieldValueBuffer : T();
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& value)
		{
			SetFieldValueInternal(name, &value);
		}

	private:
		bool GetFieldValueInternal(const std::string& name) const;
		void SetFieldValueInternal(const std::string& name, const void* value) const;

	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;

		inline static uint8_t s_FieldValueBuffer[8];
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAppAssembly(const std::filesystem::path& filepath);

		static bool EntityClassExists(const std::string& fullClassName);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
		static MonoImage* GetCoreAssemblyImage();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID uuid);

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptRegistry;
	};
}
