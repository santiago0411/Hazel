#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/FileSystem.h"

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

		static Ref<Project> New();
		static Ref<Project> Load(const FilePath& path);
		static bool SaveActive(const FilePath& path);

	private:
		ProjectConfig m_Config;
		FilePath m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
}