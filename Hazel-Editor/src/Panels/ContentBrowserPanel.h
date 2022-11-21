#pragma once

#include "Hazel/Core/FileSystem.h"

#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void OnImGuiRender();

	private:
		FilePath m_BaseDirectory, m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon, m_FileIcon;
	};
}
