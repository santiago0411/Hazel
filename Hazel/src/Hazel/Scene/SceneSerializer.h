#pragma once

#include "Hazel/Core/FileSystem.h"
#include "Hazel/Scene/Scene.h"

namespace Hazel
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const FilePath& filepath);
		void SerializeRuntime(const FilePath& filepath);

		bool Deserialize(const FilePath& filepath) const;
		bool DeserializeRuntime(const FilePath& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}
