#include "hzpch.h"
#include "Hazel/Scripting/ScriptingEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#include "Hazel/Scripting/Debug.h"
#include "Hazel/Scripting/ScriptingEngineRegistry.h"
#include "Hazel/Scene/Scene.h"

namespace Hazel
{
	struct MonoContext
	{
		MonoDomain* CurrentDomain = nullptr;
		MonoDomain* NewDomain = nullptr;
		const char* EngineAssemblyPath;
		Ref<Scene> SceneContext;

		MonoAssembly* EngineAssembly = nullptr;
	};

	MonoImage* s_EngineAssemblyImage = nullptr;

	static MonoContext* s_Context = nullptr;
	static EntityInstanceMap s_EntityInstanceMap;

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc);

	struct EntityScriptClass
	{
		std::string FullName;
		std::string ClassName;
		std::string NamespaceName;

		MonoClass* Class = nullptr;
		MonoMethod* Constructor = nullptr;
		MonoMethod* OnCreateMethod = nullptr;
		MonoMethod* OnDestroyMethod = nullptr;
		MonoMethod* OnUpdateMethod = nullptr;
		MonoMethod* OnPhysicsUpdateMethod = nullptr;

		void InitClassMethods(MonoImage* image)
		{
			Constructor = GetMethod(image, "Hazel.Entity:.ctor(ulong)");
			OnCreateMethod = GetMethod(image, FullName + ":OnCreate()");
			OnDestroyMethod = GetMethod(image, FullName + ":OnDestroy()");
			OnUpdateMethod = GetMethod(image, FullName + ":OnUpdate(single)");
			OnPhysicsUpdateMethod = GetMethod(image, FullName + ":OnPhysicsUpdate(single)");
		}
	};

	static std::unordered_map<std::string, EntityScriptClass> s_EntityClassMap;

	MonoObject* EntityInstance::GetInstance() const
	{
		HZ_CORE_ASSERT(Handle, "Entity has no been instantiated!");
		return mono_gchandle_get_target(Handle);
	}

	bool EntityInstance::IsRuntimeAvailable() const
	{
		return Handle != 0;
	}

	void ScriptingEngine::Init(const char* engineAssemblyPath)
	{
		HZ_CORE_ASSERT(!s_Context);

		s_Context = new MonoContext;

		mono_set_assemblies_path("mono/lib");
		s_Context->CurrentDomain = mono_jit_init("Hazel");
		HZ_CORE_ASSERT(s_Context->CurrentDomain, "Failed to initialize mono domain");

		s_Context->EngineAssembly = mono_domain_assembly_open(s_Context->CurrentDomain, engineAssemblyPath);
		s_Context->EngineAssemblyPath = engineAssemblyPath;
		HZ_CORE_ASSERT(s_Context->EngineAssembly, "Failed to open Hazel-ScriptCore.dll");

		s_EngineAssemblyImage = mono_assembly_get_image(s_Context->EngineAssembly);
		HZ_CORE_ASSERT(s_EngineAssemblyImage, "Failed to get image from assembly");

		RegisterMethods();
		ScriptingEngineRegistry::RegisterAll();
	}

	void ScriptingEngine::Shutdown()
	{
		if (s_EngineAssemblyImage)
			mono_image_close(s_EngineAssemblyImage);

		if (s_Context->EngineAssembly)
			mono_assembly_close(s_Context->EngineAssembly);

		if (s_Context->CurrentDomain)
			mono_jit_cleanup(s_Context->CurrentDomain);

		s_EntityInstanceMap.clear();
		delete s_Context;
	}

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc)
	{
		HZ_CORE_VERIFY(image);
		MonoMethodDesc* desc = mono_method_desc_new(methodDesc.c_str(), NULL);
		if (!desc)
			HZ_CORE_ERROR("[ScriptingEngine] mono_method_desc_new failed ({0})", methodDesc);

		MonoMethod* method = mono_method_desc_search_in_image(desc, image);
		if (!method)
			HZ_CORE_WARN("[ScriptingEngine] mono_method_desc_search_in_image failed ({0})", methodDesc);

		return method;
	}

	void ScriptingEngine::OnSceneDestruct(UUID sceneId)
	{
		if (s_EntityInstanceMap.find(sceneId) != s_EntityInstanceMap.end())
		{
			s_EntityInstanceMap.at(sceneId).clear();
			s_EntityInstanceMap.erase(sceneId);
		}
	}

	static std::unordered_map<std::string, MonoClass*> s_Classes;
	void ScriptingEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		s_Classes.clear();
		if (!scene)
			s_EntityInstanceMap.clear();
		s_Context->SceneContext = scene;
			
	}

	Ref<Scene>& ScriptingEngine::GetCurrentSceneContext()
	{
		return s_Context->SceneContext;
	}


	void ScriptingEngine::RunStaticMethod(const char* methodName)
	{
		MonoMethodDesc* methodDesc = mono_method_desc_new(methodName, NULL);
		if (!methodDesc)
		{
			HZ_CORE_ERROR("Failed to run HazelEngine.dll method: '{0}'", methodName);
			return;
		}

		MonoMethod* method = mono_method_desc_search_in_image(methodDesc, s_EngineAssemblyImage);
		if (!method)
		{
			HZ_CORE_ERROR("Failed to find the method '{0}' in HazelEngine.dll image.", methodName);
			return;
		}

		HZ_CORE_TRACE("Running static method 'HazelEngine.{0}'...", methodName);
		mono_runtime_invoke(method, nullptr, nullptr, nullptr);

		mono_method_desc_free(methodDesc);
	}

	static uint8_t GetFieldSize(FieldType type)
	{
		switch (type)
		{
			case FieldType::Bool:			return 1;
			case FieldType::Float:			return 4;
			case FieldType::Int:			return 4;
			case FieldType::UnsignedInt:	return 4;
			case FieldType::String:			return 8;
			case FieldType::Vec2:			return 4 * 2;
			case FieldType::Vec3:			return 4 * 3;
			case FieldType::Vec4:			return 4 * 4;
			case FieldType::Asset:			return 8;
			case FieldType::Entity:			return 8;
		}

		HZ_CORE_ASSERT(false);
		return 0;
	}

	PublicField::PublicField(const std::string& name, const std::string& typeName, FieldType type, bool isReadOnly)
		: Name(name), TypeName(typeName), Type(type), IsReadOnly(isReadOnly)
	{
		if (Type != FieldType::String)
			m_StoredValueBuffer = AllocateBuffer(Type);
	}

	PublicField::PublicField(const PublicField& other)
		: Name(other.Name), TypeName(other.TypeName), Type(other.Type), IsReadOnly(other.IsReadOnly)
	{
		if (Type != FieldType::String)
		{
			m_StoredValueBuffer = AllocateBuffer(Type);
			memcpy(m_StoredValueBuffer, other.m_StoredValueBuffer, GetFieldSize(Type));
		}
		else
		{
			m_StoredValueBuffer = other.m_StoredValueBuffer;
		}

		m_MonoClassField = other.m_MonoClassField;
		m_MonoProperty = other.m_MonoProperty;
	}

	PublicField::PublicField(PublicField&& other)
		: Name(std::move(other.Name)), TypeName(std::move(other.TypeName)), Type(other.Type), IsReadOnly(other.IsReadOnly),
			m_MonoClassField(other.m_MonoClassField), m_MonoProperty(other.m_MonoProperty), m_StoredValueBuffer(other.m_StoredValueBuffer)
	{
		other.m_MonoClassField = nullptr;
		other.m_MonoProperty = nullptr;
		if (Type != FieldType::String)
			other.m_StoredValueBuffer = nullptr;
	}

	PublicField::~PublicField()
	{
		if (Type != FieldType::String)
			delete[] m_StoredValueBuffer;
	}

	PublicField& PublicField::operator=(const PublicField& other)
	{
		if (&other != this)
		{
			Name = other.Name;
			TypeName = other.TypeName;
			Type = other.Type;
			IsReadOnly = other.IsReadOnly;
			m_MonoClassField = other.m_MonoClassField;
			m_MonoProperty = other.m_MonoProperty;
			if (Type != FieldType::String)
			{
				m_StoredValueBuffer = AllocateBuffer(Type);
				memcpy(m_StoredValueBuffer, other.m_StoredValueBuffer, GetFieldSize(Type));
			}
			else
			{
				m_StoredValueBuffer = other.m_StoredValueBuffer;
			}
		}

		return *this;
	}

	void PublicField::CopyStoredValueToRuntime(EntityInstance & entityInstance)
	{
		
	}

	void PublicField::CopyStoredValueFromRuntime(EntityInstance & entityInstance)
	{
		
	}

	void PublicField::SetStoredValueRaw(void* src)
	{
		if (IsReadOnly)
			return;

		uint8_t size = GetFieldSize(Type);
		memcpy(m_StoredValueBuffer, src, size);
	}

	void PublicField::SetRuntimeValueRaw(EntityInstance& entityInstance, void* src)
	{
		HZ_CORE_ASSERT(entityInstance.GetInstance());

		if (IsReadOnly)
			return;

		if (m_MonoProperty)
		{
			void* data[] = { src };
			mono_property_set_value(m_MonoProperty, entityInstance.GetInstance(), data, nullptr);
		}
		else
		{
			mono_field_set_value(entityInstance.GetInstance(), m_MonoClassField, src);
		}
;	}

	void* PublicField::GetRuntimeValueRaw(EntityInstance& entityInstance)
	{
		HZ_CORE_ASSERT(entityInstance.GetInstance());

		uint8_t* outValue = nullptr;
		mono_field_get_value(entityInstance.GetInstance(), m_MonoClassField, outValue);
		return outValue;
	}

	uint8_t* PublicField::AllocateBuffer(FieldType type)
	{
		uint8_t size = GetFieldSize(Type);
		uint8_t* buffer = new uint8_t[size];
		memset(buffer, 0, size);
		return buffer;
	}

	void PublicField::SetStoredValue_Internal(void* value)
	{
		if (IsReadOnly)
			return;

		uint8_t size = GetFieldSize(Type);
		memcpy(m_StoredValueBuffer, value, size);
	}

	void PublicField::GetStoredValue_Internal(void* outValue) const
	{
		uint8_t size = GetFieldSize(Type);
		memcpy(outValue, m_StoredValueBuffer, size);
	}

	void PublicField::SetRuntimeValue_Internal(EntityInstance& entityInstance, void* value)
	{
		HZ_CORE_ASSERT(entityInstance.GetInstance());

		if (IsReadOnly)
			return;

		if (m_MonoProperty)
		{
			void* data[] = { value };
			mono_property_set_value(m_MonoProperty, entityInstance.GetInstance(), data, nullptr);
		}
		else
		{
			mono_field_set_value(entityInstance.GetInstance(), m_MonoClassField, value);
		}
	}

	void PublicField::SetRuntimeValue_Internal(EntityInstance& entityInstance, const std::string& value)
	{
		HZ_CORE_ASSERT(entityInstance.GetInstance());

		if (IsReadOnly)
			return;

		MonoString* monoString = mono_string_new(mono_domain_get(), value.c_str());

		if (m_MonoProperty)
		{
			void* data[] = { monoString };
			mono_property_set_value(m_MonoProperty, entityInstance.GetInstance(), data, nullptr);
		}
		else
		{
			mono_field_set_value(entityInstance.GetInstance(), m_MonoClassField, monoString);
		}
	}

	void PublicField::GetRuntimeValue_Internal(EntityInstance& entityInstance, void* outValue) const
	{
		HZ_CORE_ASSERT(entityInstance.GetInstance());

		if (Type == FieldType::Entity)
		{
			MonoObject* obj;
			if (m_MonoProperty)
				obj = mono_property_get_value(m_MonoProperty, entityInstance.GetInstance(), nullptr, nullptr);
			else
				mono_field_get_value(entityInstance.GetInstance(), m_MonoClassField, &obj);

			MonoProperty* idProperty = mono_class_get_property_from_name(entityInstance.ScriptClass->Class, "ID");
			MonoObject* idObject = mono_property_get_value(idProperty, obj, nullptr, nullptr);
			memcpy(outValue, mono_object_unbox(idObject), GetFieldSize(Type));
		}
		else
		{
			if (m_MonoProperty)
			{
				MonoObject* result = mono_property_get_value(m_MonoProperty, entityInstance.GetInstance(), nullptr, nullptr);
				memcpy(outValue, mono_object_unbox(result), GetFieldSize(Type));
			}
			else
			{
				mono_field_get_value(entityInstance.GetInstance(), m_MonoClassField, outValue);
			}
		}
	}

	void PublicField::GetRuntimeValue_Internal(EntityInstance& entityInstance, std::string& outValue) const
	{
		HZ_CORE_ASSERT(entityInstance.GetInstance());

		MonoString* monoString;
		if (m_MonoProperty)
			monoString = (MonoString*)mono_property_get_value(m_MonoProperty, entityInstance.GetInstance(), nullptr, nullptr);
		else
			mono_field_get_value(entityInstance.GetInstance(), m_MonoClassField, &monoString);

		outValue = mono_string_to_utf8(monoString);
	}

	void ScriptingEngine::Test()
	{
		MonoClass* cls = mono_class_from_name(s_EngineAssemblyImage, "HazelEngine", "Tests");
		void* it = nullptr;
		MonoMethod* method;
		while (method = mono_class_get_methods(cls, &it))
		{
			HZ_CORE_INFO("Found method '{0}' in class HazelEngine.Debug", mono_method_full_name(method, 1));
		}
	}

	void ScriptingEngine::RegisterMethods()
	{
		Scripting::Debug::RegisterMethods();
	}
}
