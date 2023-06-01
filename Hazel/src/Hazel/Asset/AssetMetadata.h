#pragma once

#include "Hazel/Core/FileSystem.h"

namespace Hazel
{
	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		FilePath FilePath;

		operator bool() const { return Type != AssetType::None; }
	};
}