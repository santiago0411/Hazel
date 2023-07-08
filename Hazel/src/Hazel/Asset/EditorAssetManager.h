#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"

namespace Hazel
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>; 

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		Ref<Asset> GetAsset(AssetHandle handle) override;

		bool IsAssetHandleValid(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;
		AssetType GetAssetType(AssetHandle handle) const override;

		void ImportAsset(const FilePath& path);

		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		const FilePath& GetFilePath(AssetHandle handle) const;

		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();
	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;

		// TODO memory only assets
	};
}
