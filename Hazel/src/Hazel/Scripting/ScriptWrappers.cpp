#include "hzpch.h"
#include "ScriptWrappers.h"

#include <mono/jit/jit.h>

#include "Hazel/Core/Application.h"
#include "Hazel/Scene/Scene.h"

#include "Hazel/Scripting/ScriptingEngine.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Hazel
{
	extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	extern std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;
}

namespace Hazel::Scripting
{
	static auto GetEntity(uint64_t entityId)
	{
		Ref<Scene> scene = ScriptingEngine::GetCurrentSceneContext();
		HZ_CORE_ASSERT(scene);
		const auto& entityMap = scene->GetEntityMap();
		HZ_CORE_ASSERT(entityMap.find(entityId) != entityMap.end());
		return entityMap.at(entityId);
	}

	bool Hazel_Input_IsKeyPressed(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	bool Hazel_Input_IsMouseButtonPressed(MouseCode button)
	{
		bool wasPressed = Input::IsMouseButtonPressed(button);

		bool enableImGui = Application::Get().GetSpecification().EnableImGui;
		if (wasPressed && enableImGui && GImGui->HoveredWindow != nullptr)
		{
			// Check that the mouse is over the viewport panel
			ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");
			if (viewportWindow)
				wasPressed = GImGui->HoveredWindow->ID == viewportWindow->ID;

		}

		return wasPressed;
	}

	void Hazel_Input_GetMousePosition(glm::vec2* outPosition)
	{
		*outPosition = Input::GetMousePosition();
	}

	uint64_t Hazel_Entity_CreateEntity()
	{
		Ref<Scene> scene = ScriptingEngine::GetCurrentSceneContext();
		return scene->CreateEntity("Unnamed from C#").GetUUID();
	}

	uint64_t Hazel_Entity_Instantiate(uint64_t entityId, uint64_t prefabId)
	{
		return 0;
	}

	uint64_t Hazel_Entity_Entity_InstantiateWithPosition(uint64_t entityId, uint64_t prefabId, glm::vec3* inPosition)
	{
		return 0;
	}

	uint64_t Hazel_Entity_Entity_InstantiateWithTransform(uint64_t entityId, uint64_t prefabId, glm::vec3* inPosition, glm::vec3* inRotation, glm::vec3* inScale)
	{
		return 0;
	}

	void Hazel_Entity_DestroyEntity(uint64_t entityId)
	{
		auto entity = GetEntity(entityId);
		Ref<Scene> scene = ScriptingEngine::GetCurrentSceneContext();
		scene->SubmitToDestroyEntity(entity);
	}

	void Hazel_Entity_CreateComponent(uint64_t entityId, void* type)
	{
		auto entity = GetEntity(entityId);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		s_CreateComponentFuncs[monoType](entity);
	}

	bool Hazel_Entity_HasComponent(uint64_t entityId, void* type)
	{
		auto entity = GetEntity(entityId);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		bool result = s_HasComponentFuncs[monoType](entity);
		return result;
	}

	uint64_t Hazel_Entity_FindEntityByTag(MonoString* tag)
	{
		Ref<Scene> scene = ScriptingEngine::GetCurrentSceneContext();
		HZ_CORE_ASSERT(scene);

		Entity entity = scene->FindEntityByTag(mono_string_to_utf8(tag));
		if (entity)
			return entity.GetComponent<IdComponent>().Id;

		return 0;
	}

	MonoString* Hazel_TagComponent_GetTag(uint64_t entityId)
	{
		auto entity = GetEntity(entityId);
		auto& tagComponent = entity.GetComponent<TagComponent>();
		return mono_string_new(mono_domain_get(), tagComponent.Tag.c_str());
	}

	void Hazel_TagComponent_SetTag(uint64_t entityId, MonoString* tag)
	{
		auto entity = GetEntity(entityId);
		auto& tagComponent = entity.GetComponent<TagComponent>();
		tagComponent.Tag = mono_string_to_utf8(tag);
	}

	void Hazel_TransformComponent_GetTransform(uint64_t entityId, TransformComponent* outTransform)
	{
		auto entity = GetEntity(entityId);
		*outTransform = entity.GetComponent<TransformComponent>();
	}

	void Hazel_TransformComponent_SetTransform(uint64_t entityId, TransformComponent* inTransform)
	{
		auto entity = GetEntity(entityId);
		entity.GetComponent<TransformComponent>() = *inTransform;
	}

	void Hazel_TransformComponent_GetPosition(uint64_t entityId, glm::vec3* outPosition)
	{
		auto entity = GetEntity(entityId);
		*outPosition = entity.GetComponent<TransformComponent>().Position;
	}

	void Hazel_TransformComponent_SetPosition(uint64_t entityId, glm::vec3* inPosition)
	{
		auto entity = GetEntity(entityId);
		entity.GetComponent<TransformComponent>().Position = *inPosition;
	}

	void Hazel_TransformComponent_GetRotation(uint64_t entityId, glm::vec3* outRotation)
	{
		auto entity = GetEntity(entityId);
		*outRotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	void Hazel_TransformComponent_SetRotation(uint64_t entityId, glm::vec3* inRotation)
	{
		auto entity = GetEntity(entityId);
		entity.GetComponent<TransformComponent>().Rotation = *inRotation;
	}

	void Hazel_TransformComponent_GetScale(uint64_t entityId, glm::vec3* outScale)
	{
		auto entity = GetEntity(entityId);
		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	void Hazel_TransformComponent_SetScale(uint64_t entityId, glm::vec3* inScale)
	{
		auto entity = GetEntity(entityId);
		entity.GetComponent<TransformComponent>().Scale = *inScale;
	}

	void Hazel_TransformComponent_GetWorldSpaceTransform(uint64_t entityId, TransformComponent* outTransform)
	{
		auto entity = GetEntity(entityId);
		Ref<Scene> scene = ScriptingEngine::GetCurrentSceneContext();
		HZ_CORE_ASSERT(scene);
		*outTransform = scene->GetWorldSpaceTransform(entity);
	}

	MonoObject* Hazel_ScriptComponent_GetInstance(uint64_t entityId)
	{
		Ref<Scene> scene = ScriptingEngine::GetCurrentSceneContext();
		HZ_CORE_ASSERT(scene);

		return nullptr;
	}
}
