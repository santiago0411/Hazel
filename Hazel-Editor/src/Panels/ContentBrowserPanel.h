#pragma once

#include "Hazel/Core/FileSystem.h"

#include "Hazel/Renderer/Texture.h"

#include <map>

namespace Hazel
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void OnImGuiRender();

	private:
		void RefreshAssetTree();

	private:
		FilePath m_BaseDirectory, m_CurrentDirectory;
		Ref<Texture2D> m_DirectoryIcon, m_FileIcon;

		struct TreeNode
		{
			FilePath Path;

			uint32_t Parent = -1;
			std::map<FilePath, uint32_t> Children;

			TreeNode(const FilePath& path)
				: Path(path) {}
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
