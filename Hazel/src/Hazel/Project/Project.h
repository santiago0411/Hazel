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

		AssetHandle StartScene;

		FilePath AssetDirectory;
		FilePath AssetRegistryPath;
		FilePath ScriptModulePath;
	};

	class Project
	{
	public:
		const FilePath& GetProjectDirectory()
		{
			return m_ProjectDirectory;
		}

		FilePath GetAssetDirectory()
		{
			return GetProjectDirectory() / m_Config.AssetDirectory;
		}

		FilePath GetAssetRegistryPath()
		{
			return GetAssetDirectory() / m_Config.AssetRegistryPath;
		}

		// TODO move to asset manager
		FilePath GetAssetFileSystemPath(const FilePath& path)
		{
			return GetAssetDirectory() / path;
		}

		FilePath GetAssetAbsolutePath(const FilePath& path)
		{
			return GetAssetDirectory() / path;
		}

		static const FilePath& GetActiveProjectDirectory()
		{
			HZ_CORE_ASSERT(s_ActiveProject)
			return s_ActiveProject->GetProjectDirectory();
		}

		static FilePath GetActiveAssetDirectory()
		{
			HZ_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetDirectory();
		}

		static FilePath GetActiveAssetRegistryPath()
		{
			HZ_CORE_ASSERT(s_ActiveProject)
			return s_ActiveProject->GetAssetRegistryPath();
		}

		// TODO move to asset manager
		static FilePath GetActiveAssetFileSystemPath(const FilePath& path)
		{
			HZ_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetActiveAssetFileSystemPath(path);
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
