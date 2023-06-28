#pragma once

#include "Hazel/Core/UUID.h"

namespace Hazel
{
	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Texture2D,
	};

	namespace Utils
	{
		static std::string AssetTypeToString(const AssetType type)
		{
			switch (type)
			{
				case AssetType::None:		return "None";
				case AssetType::Scene:		return "Scene";
				case AssetType::Texture2D:	return "Texture2D";
			}

			return "Unknown AssetType";
		}

		static AssetType AssetTypeFromString(std::string_view type)
		{
			if (type == "Scene")		return AssetType::Scene;
			if (type == "Texture2D")	return AssetType::Texture2D;

			return AssetType::None;
		}
	}

	class Asset
	{
	public:
		AssetHandle Handle;

		virtual AssetType GetType() const = 0;
	};
}
