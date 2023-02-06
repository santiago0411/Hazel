#pragma once

#include "Hazel/Core/FileSystem.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	struct MSDFData;

	class Font
	{
	public:
		Font(const FilePath& fontPath);
		~Font();

		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

	private:
		MSDFData* m_Data = nullptr;
		Ref<Texture2D> m_AtlasTexture;
	};
}
