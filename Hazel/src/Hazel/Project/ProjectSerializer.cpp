#include "hzpch.h"
#include "ProjectSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Hazel
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	bool ProjectSerializer::Serialize(const FilePath& path)
	{
		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap; // Root
		out << YAML::Key << "Project" << YAML::Value << YAML::BeginMap; // Project
		{
			out << YAML::Key << "Name" << YAML::Value << config.Name;
			out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
			out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
			out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.AssetRegistryPath.string();
			out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
		}
		out << YAML::EndMap; // Project
		out << YAML::EndMap; // Root

		std::ofstream fout(path);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const FilePath& path)
	{
		auto& config = m_Project->GetConfig();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException& e)
		{
			HZ_CORE_ERROR("Error parsing project file. {0}", e.msg);
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<std::string>();
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		if (projectNode["AssetRegistryPath"])
			config.AssetRegistryPath = projectNode["AssetRegistryPath"].as<std::string>();
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();

		return true;
	}
}
