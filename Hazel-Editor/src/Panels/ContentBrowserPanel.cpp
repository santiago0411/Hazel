#include "ContentBrowserPanel.h"

#include "Hazel/Asset/TextureImporter.h"

#include "Hazel/Project/Project.h"

#include "Hazel/Utils/StringUtils.h"

#include <imgui/imgui.h>

namespace Hazel
{
	namespace Fs = std::filesystem;

	ContentBrowserPanel::ContentBrowserPanel(Ref<Project> project)
		: m_Project(project), m_ThumbnailCache(CreateRef<ThumbnailCache>(project)),
			m_BaseDirectory(m_Project->GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		m_TreeNodes.emplace_back(".", 0);

		m_DirectoryIcon = TextureImporter::LoadTexture2D("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = TextureImporter::LoadTexture2D("Resources/Icons/ContentBrowser/FileIcon.png");

		RefreshAssetTree();

		m_Mode = Mode::FileSystem;
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		const char* label = m_Mode == Mode::Asset ? "Asset" : "File";
		if (ImGui::Button(label))
			m_Mode = m_Mode == Mode::Asset ? Mode::FileSystem : Mode::Asset;

		if (m_CurrentDirectory != m_BaseDirectory)
		{
			ImGui::SameLine();
			if (ImGui::Button("<-"))
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}

		static float padding = 0;
		static float thumbnailSize = 100 ;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int32_t columnCount = (int32_t)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		if (m_Mode == Mode::Asset)
		{
			TreeNode* node = &m_TreeNodes[0];

			FilePath currentDir = Fs::relative(m_CurrentDirectory, Project::GetActiveAssetDirectory());
			for (const auto& p : currentDir)
			{
				// If only one level
				if (node->Path == currentDir)
					break;

				if (node->Children.find(p) != node->Children.end())
				{
					node = &m_TreeNodes[node->Children[p]];
					continue;
				}

				// Couldn't find path
				HZ_CORE_ASSERT(false)
			}

			for (const auto& [item, treeNodeIndex] : node->Children)
			{
				bool isDirectory = Fs::is_directory(Project::GetActiveAssetDirectory() / item);
				std::string itemStr = item.generic_string();

				ImGui::PushID(itemStr.c_str());

				Ref<Texture2D> icon = isDirectory ? m_DirectoryIcon : m_FileIcon;

				ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
				ImGui::ImageButton((ImTextureID)icon->GetRendererId(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

				if (ImGui::BeginDragDropSource())
				{
					AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle; 
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &handle, sizeof(AssetHandle), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				ImGui::PopStyleColor();
				if (ImGui::IsItemHovered() 
					&& ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
					&& isDirectory)
				{
					m_CurrentDirectory /= item.filename();
				}

				ImGui::TextWrapped(itemStr.c_str());
				ImGui::NextColumn();
				ImGui::PopID();
			}
		}
		else
		{
			uint32_t count = 0;
			for (auto& directoryEntry : Fs::directory_iterator(m_CurrentDirectory))
				count++;

			ImGuiListClipper clipper(glm::ceil(count / (float)columnCount));
			bool first = true;
			while (clipper.Step())
			{
				auto it = Fs::directory_iterator(m_CurrentDirectory);
				if (!first)
				{
					for (int i = 0; i < clipper.DisplayStart; i++)
					{
						for (int c = 0; c < columnCount && it != Fs::directory_iterator(); c++)
						{
							it++;
						}
					}
				}

				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					int c;
					for (c = 0; c < columnCount && it != Fs::directory_iterator(); c++, it++)
					{
						const auto& directoryEntry = *it;
						const auto& path = directoryEntry.path();
						std::string filenameString = path.filename().string();

						ImGui::PushID(filenameString.c_str());

						auto relativePath = Fs::relative(path, Project::GetActiveAssetDirectory());
						Ref<Texture2D> thumbnail = m_DirectoryIcon;
						if (!directoryEntry.is_directory())
						{
							thumbnail = m_ThumbnailCache->GetOrCreateThumbnail(relativePath);
							if (!thumbnail)
								thumbnail = m_FileIcon;
						}

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						float thumbnailHeight = thumbnailSize * ((float)thumbnail->GetHeight() / (float)thumbnail->GetWidth());
						float diff = thumbnailSize - thumbnailHeight;

						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff);

						ImGui::ImageButton((ImTextureID)(uint64_t)thumbnail->GetRendererId(), { thumbnailSize, thumbnailHeight }, { 0, 1 }, { 1, 0 });
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							std::string sizeString = Utils::BytesToString(thumbnail->GetEstimatedSize());
							ImGui::Text("Mem: %s", sizeString.c_str());
							ImGui::EndTooltip();
						}

						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::MenuItem("Import"))
							{
								Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
								RefreshAssetTree();
							}
							ImGui::EndPopup();
						}

						ImGui::PopStyleColor();

						if (ImGui::IsItemHovered()
							&& ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
							&& directoryEntry.is_directory())
						{
							m_CurrentDirectory /= path.filename();
						}

						ImGui::TextWrapped(filenameString.c_str());
						ImGui::NextColumn();
						ImGui::PopID();
					}

					if (first && c < columnCount)
						for (int extra = 0; extra < columnCount - c; extra++)
							ImGui::NextColumn();
				}

				first = false;
			}

#if 0
			for (auto& directoryEntry : Fs::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				std::string filenameString = path.filename().string();

				ImGui::PushID(filenameString.c_str());

				auto relativePath = Fs::relative(path, Project::GetActiveAssetDirectory());
				Ref<Texture2D> thumbnail = m_DirectoryIcon;
				if (!directoryEntry.is_directory())
				{
					thumbnail = m_ThumbnailCache->GetOrCreateThumbnail(relativePath);
					if (!thumbnail)
						thumbnail = m_FileIcon;
				}

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::ImageButton((ImTextureID)(uint64_t)thumbnail->GetRendererId(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Import"))
					{
						Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
						RefreshAssetTree();
					}
					ImGui::EndPopup();
				}

				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered()
					&& ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
					&& directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
				}

				ImGui::TextWrapped(filenameString.c_str());
				ImGui::NextColumn();
				ImGui::PopID();
			}
#endif
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();

		m_ThumbnailCache->OnUpdate();
	}

	void ContentBrowserPanel::RefreshAssetTree()
	{
		const AssetRegistry& registry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		for (const auto& [handle, metadata] : registry)
		{
			uint32_t currentNodeIndex = 0;

			for (const auto& p : metadata.FilePath)
			{
				auto it = m_TreeNodes[currentNodeIndex].Children.find(p.generic_string());
				if (it != m_TreeNodes[currentNodeIndex].Children.end())
				{
					currentNodeIndex = it->second;
				}
				else
				{
					TreeNode newNode(p, handle);
					newNode.Parent = currentNodeIndex;
					m_TreeNodes.push_back(newNode);

					m_TreeNodes[currentNodeIndex].Children[p] = m_TreeNodes.size() - 1;
					currentNodeIndex = m_TreeNodes.size() - 1;
				}
			}
		}
	}
}
