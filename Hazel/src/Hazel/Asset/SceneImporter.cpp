#include "hzpch.h"
#include "SceneImporter.h"

#include "Hazel/Project/Project.h"
#include "Hazel/Scene/SceneSerializer.h"

namespace Hazel
{
	Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
	{
		HZ_PROFILE_FUNCTION();
		return LoadScene(Project::GetAssetDirectory() / metadata.FilePath);
	}

	Ref<Scene> SceneImporter::LoadScene(const FilePath& path)
	{
		HZ_PROFILE_FUNCTION();

		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(path);
		return scene;
	}

	void SceneImporter::SaveScene(Ref<Scene> scene, const FilePath& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(Project::GetAssetDirectory() / path);
	}
}
