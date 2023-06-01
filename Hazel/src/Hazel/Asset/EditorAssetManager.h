#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"

namespace Hazel
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>; 

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		Ref<Asset> GetAsset(AssetHandle handle) const override;

		bool IsAssetHandleValid(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;

		const AssetMetadata& GetMetadata(AssetHandle handle) const;

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;

		// TODO memory only assets
	};
}
