#include "hzpch.h"
#include "ScriptEngine.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Core/FileSystem.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scripting/ScriptRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/threads.h>

#include "FileWatch.hpp"

namespace Hazel
{
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Boolean",	ScriptFieldType::Boolean	},
		{ "System.Byte",	ScriptFieldType::Byte		},
		{ "System.SByte",	ScriptFieldType::SByte		},
		{ "System.UInt16",	ScriptFieldType::UShort		},
		{ "System.Int16",	ScriptFieldType::Short		},
		{ "System.UInt32",	ScriptFieldType::UInt		},
		{ "System.Int32",	ScriptFieldType::Int		},
		{ "System.UInt64",	ScriptFieldType::ULong		},
		{ "System.Int64",	ScriptFieldType::Long		},
		{ "System.Single",	ScriptFieldType::Float		},
		{ "System.Double",	ScriptFieldType::Double		},
		{ "System.Decimal",	ScriptFieldType::Decimal	},
		{ "System.Char",	ScriptFieldType::Char		},
		{ "System.String",	ScriptFieldType::String		},
		{ "Hazel.Vector2",	ScriptFieldType::Vector2	},
		{ "Hazel.Vector3",	ScriptFieldType::Vector3	},
		{ "Hazel.Vector4",	ScriptFieldType::Vector4	},
		{ "Hazel.Color",	ScriptFieldType::Color		},
		{ "Hazel.Entity",	ScriptFieldType::Entity		},
	};

	namespace Utils
	{
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);
			if (!fileData)
			{
				HZ_CORE_ERROR("Failed to read assembly: {0}", assemblyPath.string());
				return nullptr;
			}

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), (uint32_t)fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				HZ_CORE_ERROR(errorMessage);
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
					if (pdbFileData)
					{
						mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), (int32_t)pdbFileData.Size());
						HZ_CORE_INFO("Loaded PDB: {0}", pdbPath.string());
					}
					else
					{
						HZ_CORE_WARN("Failed to load PDB: {0}", pdbPath.string());
					}
				}
			}

			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			HZ_CORE_INFO("Loaded assembly: {0}", assemblyPath.string());
			return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTables = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			const int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTables);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTables, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				HZ_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			const auto it = s_ScriptFieldTypeMap.find(typeName);
 			if (it == s_ScriptFieldTypeMap.end())
				return ScriptFieldType::None;

			return it->second;
		}
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage * CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

		bool EnableDebugging = true;

		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (changeType == filewatch::Event::modified && !s_Data->AssemblyReloadPending)
		{
			s_Data->AssemblyReloadPending = true;

			Application::Get().SubmitToMainThread([] 
			{
				s_Data->AppAssemblyFileWatcher.reset();
				ScriptEngine::ReloadAssembly();
			});
		}
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData;

		InitMono();
		ScriptRegistry::RegisterMethods();

		if (!LoadAssembly("Resources/Scripts/Hazel-ScriptCore.dll"))
			return;

		if (!LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll"))
			return;

		LoadAssemblyClasses();
		ScriptRegistry::RegisterComponents();

		MonoClass* entityMonoClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Hazel", "Entity");
		s_Data->EntityClass = ScriptClass(entityMonoClass);
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		if (s_Data->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(std::size(argv), (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain);
		s_Data->RootDomain = rootDomain;

		if (s_Data->EnableDebugging)
			mono_debug_domain_create(s_Data->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyFilepath = filepath;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (!s_Data->CoreAssembly)
		{
			HZ_CORE_ERROR("Could not load Hazel-ScriptCore assembly.");
			return false;
		}

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		// Utils::PrintAssemblyTypes(s_Data->CoreAssembly);

		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		if (!s_Data->AppAssembly)
		{
			HZ_CORE_ERROR("Could not load app assembly.");
			return false;
		}

		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		// Utils::PrintAssemblyTypes(s_Data->AppAssembly);

		s_Data->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
		s_Data->AssemblyReloadPending = false;

		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		HZ_CORE_INFO("Reloading assemblies.");

		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_Data->AppDomain);

		if (!LoadAssembly(s_Data->CoreAssemblyFilepath))
			return;

		if (!LoadAppAssembly(s_Data->AppAssemblyFilepath))
			return;

		LoadAssemblyClasses();
		ScriptRegistry::RegisterComponents();

		MonoClass* entityMonoClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Hazel", "Entity");
		s_Data->EntityClass = ScriptClass(entityMonoClass);
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& fullClassName)
	{
		const auto it = s_Data->EntityClasses.find(fullClassName);
		if (it == s_Data->EntityClasses.end())
			return nullptr;

		return it->second;
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (EntityClassExists(sc.ClassName))
		{
			UUID entityId = entity.GetUUID();
			auto instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entityId);
			s_Data->EntityInstances[entityId] = instance;

			// Copy values
			const auto it = s_Data->EntityScriptFields.find(entityId);
			if (it != s_Data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = it->second;
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		auto it = s_Data->EntityInstances.find(entityUUID);
		if (it != s_Data->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = it->second;
			instance->InvokeOnUpdate(ts);
		}
		else
		{
			HZ_CORE_ERROR("Could not find ScriptInstance for entity {0}", entityUUID);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTables = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		const int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTables);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Hazel", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTables, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

			if (monoClass == entityClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
				continue;

			std::string fullName = strlen(nameSpace) != 0 ? fmt::format("{}.{}", nameSpace, className) : className;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;

			int fieldCount = mono_class_num_fields(monoClass);
			HZ_CORE_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					HZ_CORE_WARN("    {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));
					scriptClass->m_Fields[fieldName] = { fieldName, fieldType, field };
				}
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID uuid)
	{
		const auto it = s_Data->EntityInstances.find(uuid);
		return it == s_Data->EntityInstances.end() ? nullptr : it->second;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		HZ_CORE_ASSERT(entity); 
		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		const auto& scriptInstance = GetEntityScriptInstance(uuid);
		return scriptInstance ? scriptInstance->GetManagedObject() : nullptr;
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	ScriptClass::ScriptClass(MonoClass* cls)
		: m_MonoClass(cls)
	{
	}

	MonoObject* ScriptClass::Instantiate() const
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount) const
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** parameters)
	{
		MonoObject* exception = nullptr;
		// TODO get info from exception
		return mono_runtime_invoke(method, instance, parameters, &exception);
	}

	ScriptInstance::ScriptInstance(const Ref<ScriptClass>& scriptClass, UUID entityId)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		void* param = &entityId;
		m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
	}

	void ScriptInstance::InvokeOnCreate() const
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts) const
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name) const
	{
		const auto& fields = m_ScriptClass->GetFields();
		const auto it = fields.find(name);
		if (it == fields.end())
			return false;

		mono_field_get_value(m_Instance, it->second.Field, s_FieldValueBuffer);
		return true;
	}

	void ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value) const
	{
		const auto& fields = m_ScriptClass->GetFields();
		const auto it = fields.find(name);
		if (it != fields.end())
			mono_field_set_value(m_Instance, it->second.Field, const_cast<void*>(value));
	}
}
