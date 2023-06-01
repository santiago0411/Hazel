#include "hzpch.h"
#include "Hazel/Asset/AssetImporter.h"

#include "TextureImporter.h"

#include <map>

namespace Hazel
{
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		// { AssetType::Scene, nullptr },
		{ AssetType::Texture2D, TextureImporter::ImportTexture2D }
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
		{
			HZ_CORE_ERROR("No importer available for asset type: {}", Utils::AssetTypeToString(metadata.Type));
			return nullptr;
		}

		return s_AssetImportFunctions[metadata.Type](handle, metadata);
	}
}
