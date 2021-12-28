#pragma once

#include "Hazel/Core/Input.h"
#include "Hazel/Scene/Components.h"

#include <glm/glm.hpp>


extern "C" {
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace Hazel::Scripting
{
	// Input
	bool Hazel_Input_IsKeyPressed(KeyCode key);
	bool Hazel_Input_IsMouseButtonPressed(MouseCode button);
	void Hazel_Input_GetMousePosition(glm::vec2* outPosition);

	// Entity
	uint64_t Hazel_Entity_CreateEntity();
	uint64_t Hazel_Entity_Instantiate(uint64_t entityId, uint64_t prefabId);
	uint64_t Hazel_Entity_Entity_InstantiateWithPosition(uint64_t entityId, uint64_t prefabId, glm::vec3* inPosition);
	uint64_t Hazel_Entity_Entity_InstantiateWithTransform(uint64_t entityId, uint64_t prefabId, glm::vec3* inPosition, glm::vec3* inRotation, glm::vec3* inScale);
	void Hazel_Entity_DestroyEntity(uint64_t entityId);
	void Hazel_Entity_CreateComponent(uint64_t entityId, void* type);
	bool Hazel_Entity_HasComponent(uint64_t entityId, void* type);
	uint64_t Hazel_Entity_FindEntityByTag(MonoString* tag);

	// TagComponent
	MonoString* Hazel_TagComponent_GetTag(uint64_t entityId);
	void Hazel_TagComponent_SetTag(uint64_t entityId, MonoString* tag);

	// TransformComponent
	void Hazel_TransformComponent_GetTransform(uint64_t entityId, TransformComponent* outTransform);
	void Hazel_TransformComponent_SetTransform(uint64_t entityId, TransformComponent* inTransform);
	void Hazel_TransformComponent_GetPosition(uint64_t entityId, glm::vec3* outPosition);
	void Hazel_TransformComponent_SetPosition(uint64_t entityId, glm::vec3* inPosition);
	void Hazel_TransformComponent_GetRotation(uint64_t entityId, glm::vec3* outRotation);
	void Hazel_TransformComponent_SetRotation(uint64_t entityId, glm::vec3* inRotation);
	void Hazel_TransformComponent_GetScale(uint64_t entityId, glm::vec3* outScale);
	void Hazel_TransformComponent_SetScale(uint64_t entityId, glm::vec3* inScale);
	void Hazel_TransformComponent_GetWorldSpaceTransform(uint64_t entityId, TransformComponent* outTransform);

	// ScriptComponent
	MonoObject* Hazel_ScriptComponent_GetInstance(uint64_t entityId);
}