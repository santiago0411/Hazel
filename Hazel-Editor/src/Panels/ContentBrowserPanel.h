#pragma once

#include "Hazel/Core/FileSystem.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Project/Project.h"
#include "ThumbnailCache.h"

#include <map>


namespace Hazel
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel(Ref<Project> project);
		~ContentBrowserPanel() = default;

		void OnImGuiRender();

	private:
		void RefreshAssetTree();

	private:
		Ref<Project> m_Project;
		Ref<ThumbnailCache> m_ThumbnailCache;

		FilePath m_BaseDirectory, m_CurrentDirectory;
		Ref<Texture2D> m_DirectoryIcon, m_FileIcon;

		struct TreeNode
		{
			FilePath Path;
			AssetHandle Handle = 0;

			uint32_t Parent = -1;
			std::map<FilePath, uint32_t> Children;

			TreeNode(const FilePath& path, AssetHandle handle)
				: Path(path), Handle(handle) {}
		};

		std::vector<TreeNode> m_TreeNodes;
		std::map<FilePath, std::vector<FilePath>> m_AssetTree;

		enum class Mode
		{
			Asset = 0, FileSystem = 1
		};

		Mode m_Mode = Mode::Asset;
	};
}
