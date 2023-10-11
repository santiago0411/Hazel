#include "hzpch.h"
#include "TextureImporter.h"

#include "Hazel/Project/Project.h"

#include <stb_image.h>

namespace Hazel
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		HZ_PROFILE_FUNCTION();
		return LoadTexture2D(Project::GetActiveAssetDirectory() / metadata.FilePath);
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const FilePath& path)
	{
		HZ_PROFILE_FUNCTION();

		int32_t width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data;

		{
			HZ_PROFILE_SCOPE("stbi_load - TextureImporter::LoadTexture2D");
			std::string pathStr = path.string();
			data.Data = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);
		}

		if (data.Data == nullptr)
		{
			HZ_CORE_ERROR("TextureImporter::LoadTexture2D - Could not load texture from filepath: {}", path);
			return nullptr;
		}

		data.Size = width * height * channels;

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;
		switch (channels)
		{
			case 3:
				spec.Format = ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = ImageFormat::RGBA8;
				break;
		}

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		data.Release();
		return texture;
	}
}
