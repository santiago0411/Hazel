#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/FileSystem.h"

#include "Hazel/Asset/RuntimeAssetManager.h"
#include "Hazel/Asset/EditorAssetManager.h"

#include <string>
#include <filesystem>

namespace Hazel
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		FilePath StartScene;

		FilePath AssetDirectory;
		FilePath ScriptModulePath;
	};

	class Project
	{
	public:
		static const FilePath& GetProjectDirectory()
		{
			HZ_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		static FilePath GetAssetDirectory()
		{
			HZ_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		// TODO move to asset manager
		static FilePath GetAssetFileSystemPath(const FilePath& path)
		{
			HZ_CORE_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }
		Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() const { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); }
		Ref<EditorAssetManager> GetEditorAssetManager() const { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }

		static Ref<Project> New();
		static Ref<Project> Load(const FilePath& path);
		static bool SaveActive(const FilePath& path);

	private:
		ProjectConfig m_Config;
		FilePath m_ProjectDirectory;
		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}
