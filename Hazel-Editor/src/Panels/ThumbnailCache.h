#pragma once

#include "Hazel/Project/Project.h"

#include "Hazel/Renderer/Texture.h"

#include <map>

namespace Hazel
{
	struct ThumbnailImage
	{
		uint64_t Timestamp;
		Ref<Texture2D> Image;
	};

	class ThumbnailCache
	{
	public:
		ThumbnailCache(Ref<Project> project);

		Ref<Texture2D> GetOrCreateThumbnail(const FilePath& assetPath);

	private:
		Ref<Project> m_Project;

		std::map<FilePath, ThumbnailImage> m_CachedImages;

		// Temp
		FilePath m_ThumbnailCachePath;
	};
}
