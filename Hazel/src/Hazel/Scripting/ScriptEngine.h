#pragma once

#include "Hazel/Core/FileSystem.h"

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
	constexpr uint8_t MAX_SCRIPT_FIELD_BUFFER_SIZE = 16;

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
		MonoClassField* Field = nullptr;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;

		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= MAX_SCRIPT_FIELD_BUFFER_SIZE, "Field Type is too large!");
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= MAX_SCRIPT_FIELD_BUFFER_SIZE, "Field Type is too large!");
			memcpy_s(m_Buffer, MAX_SCRIPT_FIELD_BUFFER_SIZE, &value, sizeof(T));
		}

	private:
		uint8_t m_Buffer[MAX_SCRIPT_FIELD_BUFFER_SIZE]{0};
		friend class ScriptEngine;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

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
			static_assert(sizeof(T) <= MAX_SCRIPT_FIELD_BUFFER_SIZE, "Field Type is too large!");
			bool success = GetFieldValueInternal(name);
			return success ? *(T*)s_FieldValueBuffer : T();
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& value)
		{
			static_assert(sizeof(T) <= MAX_SCRIPT_FIELD_BUFFER_SIZE, "Field Type is too large!");
			SetFieldValueInternal(name, &value);
		}

		MonoObject* GetManagedObject() const { return m_Instance; }

	private:
		bool GetFieldValueInternal(const std::string& name) const;
		void SetFieldValueInternal(const std::string& name, const void* value) const;

	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;

		inline static uint8_t s_FieldValueBuffer[MAX_SCRIPT_FIELD_BUFFER_SIZE];

		friend class ScriptEngine;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static bool LoadAssembly(const FilePath& filepath);
		static bool LoadAppAssembly(const FilePath& filepath);

		static void ReloadAssembly();

		static bool EntityClassExists(const std::string& fullClassName);
		static Ref<ScriptClass> GetEntityClass(const std::string& fullClassName);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
		static MonoImage* GetCoreAssemblyImage();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID uuid);

		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

		static MonoObject* GetManagedInstance(UUID uuid);

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptRegistry;
	};

	namespace Utils
	{
		inline const char* ScriptFieldTypeToString(const ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
				case ScriptFieldType::None:		return "None";
				case ScriptFieldType::Boolean:	return "Boolean";
				case ScriptFieldType::Byte:		return "Byte";
				case ScriptFieldType::SByte:	return "SByte";
				case ScriptFieldType::UShort:	return "UShort";
				case ScriptFieldType::Short:	return "Short";
				case ScriptFieldType::UInt:		return "UInt";
				case ScriptFieldType::Int:		return "Int";
				case ScriptFieldType::ULong:	return "ULong";
				case ScriptFieldType::Long:		return "Long";
				case ScriptFieldType::Float:	return "Float";
				case ScriptFieldType::Double:	return "Double";
				case ScriptFieldType::Decimal:	return "Decimal";
				case ScriptFieldType::Char:		return "Char";
				case ScriptFieldType::String:	return "String";
				case ScriptFieldType::Vector2:	return "Vector2";
				case ScriptFieldType::Vector3:	return "Vector3";
				case ScriptFieldType::Vector4:	return "Vector4";
				case ScriptFieldType::Color:	return "Color";
				case ScriptFieldType::Entity:	return "Entity";
			}

			HZ_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(const std::string_view fieldType)
		{
			if (fieldType == "None")	return ScriptFieldType::None;
			if (fieldType == "Boolean")	return ScriptFieldType::Boolean;
			if (fieldType == "Byte")	return ScriptFieldType::Byte;
			if (fieldType == "SByte")	return ScriptFieldType::SByte;
			if (fieldType == "UShort")	return ScriptFieldType::UShort;
			if (fieldType == "Short")	return ScriptFieldType::Short;
			if (fieldType == "UInt")	return ScriptFieldType::UInt;
			if (fieldType == "Int")		return ScriptFieldType::Int;
			if (fieldType == "ULong")	return ScriptFieldType::ULong;
			if (fieldType == "Long")	return ScriptFieldType::Long;
			if (fieldType == "Float")	return ScriptFieldType::Float;
			if (fieldType == "Double")	return ScriptFieldType::Double;
			if (fieldType == "Decimal")	return ScriptFieldType::Decimal;
			if (fieldType == "Char")	return ScriptFieldType::Char;
			if (fieldType == "String")	return ScriptFieldType::String;
			if (fieldType == "Vector2")	return ScriptFieldType::Vector2;
			if (fieldType == "Vector3")	return ScriptFieldType::Vector3;
			if (fieldType == "Vector4")	return ScriptFieldType::Vector4;
			if (fieldType == "Color")	return ScriptFieldType::Color;
			if (fieldType == "Entity")	return ScriptFieldType::Entity;

			HZ_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}
	}
}
