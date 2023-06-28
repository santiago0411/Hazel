#include "hzpch.h"
#include "Hazel/Asset/EditorAssetManager.h"

#include "Hazel/Asset/AssetImporter.h"

#include "Hazel/Project/Project.h"

#include "Hazel/Utils/YamlUtils.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Hazel
{
	bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	void EditorAssetManager::ImportAsset(const FilePath& path)
	{
		AssetHandle handle;
		AssetMetadata metadata;
		metadata.Type = AssetType::Texture2D; // TODO grab this from extension
		metadata.FilePath = path;
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		asset->Handle = handle;
		if (asset)
		{
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			SerializeAssetRegistry();
		}
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
	{
		static AssetMetadata s_NullMetadata;
		auto it = m_AssetRegistry.find(handle);
		if (it == m_AssetRegistry.end())
			return s_NullMetadata;

		return it->second;
	}

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		if (!IsAssetHandleValid(handle))
			return nullptr;

		Ref<Asset> asset;

		if (IsAssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			const AssetMetadata& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(handle, metadata);
			if (!asset)
				HZ_CORE_ERROR("EditorAssetManager::GetMetadata - asset import failed!");
		}

		return asset;
	}

	void EditorAssetManager::SerializeAssetRegistry()
	{
		FilePath path = Project::GetAssetRegistryPath();

		YAML::Emitter out;
		out << YAML::BeginMap; // Root
		out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginSeq; // AssetRegistry

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			out << YAML::BeginMap; // Metadata
			out << YAML::Key << "Handle" << YAML::Value << handle;
			std::string filepathStr = metadata.FilePath.generic_string();
			out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
			out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(metadata.Type);
			out << YAML::EndMap; // Metadata
		}
		out << YAML::EndSeq; // AssetRegistry
		out << YAML::EndMap; // Root

		std::ofstream fout(path);
		fout << out.c_str();
	}

	bool EditorAssetManager::DeserializeAssetRegistry()
	{
		FilePath path = Project::GetAssetRegistryPath();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException& e)
		{
			HZ_CORE_ERROR("Error parsing asset registry file. {0}", e.msg);
			return false;
		}

		auto rootNode = data["AssetRegistry"];
		if (!rootNode)
			return false;

		for (const auto& node : rootNode)
		{
			auto handle = node["Handle"].as<AssetHandle>();
			AssetMetadata& metadata = m_AssetRegistry[handle];
			metadata.FilePath = node["FilePath"].as<std::string>();
			metadata.Type = Utils::AssetTypeFromString(node["Type"].as<std::string>());
		}

		return true;
	}
}
