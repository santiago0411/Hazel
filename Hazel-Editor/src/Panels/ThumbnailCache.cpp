#include "ThumbnailCache.h"

#include "Hazel/Asset/TextureImporter.h"

#include <chrono>

namespace Hazel
{
	ThumbnailCache::ThumbnailCache(Ref<Project> project)
		: m_Project(project)
	{
		m_ThumbnailCachePath = m_Project->GetAssetDirectory() / "Thumbnail.cache";
	}

	Ref<Texture2D> ThumbnailCache::GetOrCreateThumbnail(const FilePath& assetPath)
	{
		auto absolutePath = m_Project->GetAssetAbsolutePath(assetPath);
		std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(absolutePath);
		uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();

		auto it = m_CachedImages.find(assetPath);
		if (it != m_CachedImages.end())
		{
			ThumbnailImage& cachedImage = it->second;
			if (cachedImage.Timestamp == timestamp)
				return cachedImage.Image;
		}

		// TODO: PNGs only for now
		if (assetPath.extension() != ".png")
			return nullptr;

		m_Queue.push({ absolutePath, assetPath, timestamp });
		return nullptr;
	}

	void ThumbnailCache::OnUpdate()
	{
		while (!m_Queue.empty())
		{
			const auto& [absolutePath, assetPath, timestamp] = m_Queue.front();

			auto it = m_CachedImages.find(assetPath);
			if (it != m_CachedImages.end())
			{
				ThumbnailImage& cachedImage = it->second;
				if (cachedImage.Timestamp == timestamp)
				{
					m_Queue.pop();
					continue;
				}
			}

			Ref<Texture2D> thumbnail = TextureImporter::LoadTexture2D(absolutePath);
			float thumbnailHeight = m_ThumbnailSize * ((float)thumbnail->GetHeight() / (float)thumbnail->GetWidth());
			thumbnail->ChangeSize(m_ThumbnailSize, thumbnailHeight);
			if (!thumbnail)
			{
				m_Queue.pop();
				continue;
			}

			ThumbnailImage& cachedImage = m_CachedImages[assetPath];
			cachedImage.Timestamp = timestamp;
			cachedImage.Image = thumbnail;

			m_Queue.pop();
			break;
		}
	}
}
