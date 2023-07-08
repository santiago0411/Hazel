#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Asset/AssetMetadata.h"

#include "Hazel/Scene/Scene.h"

namespace Hazel
{
	class SceneImporter
	{
	public:
		// AssetMetadata filepath is relative to project asset directory
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);

		// Reads file directly from filesystem
		// (i.e. path has to be relative / absolute to working directory)
		static Ref<Scene> LoadScene(const FilePath& path);

		static void SaveScene(Ref<Scene> scene, const FilePath& path);
	};
}
