#include "hzpch.h"
#include "ProjectSerializer.h"

#include "ProjectRuntimeFormat.h"
#include "Hazel/Asset/AssetManager/EditorAssetManager.h"
#include "Hazel/Serialization/FileStream.h"

#include <cstring>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Hazel {

	ProjectSerializer::ProjectSerializer(const Ref<Project>& project)
		: m_Project(project)
	{
	}

	void ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << config.Name;
			out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory;
			out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath;
			out << YAML::Key << "StartScene" << YAML::Value << config.StartScene;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		HZ_CORE_ASSERT(fout.good(), "Failed to open project file for writing");
		fout << out.c_str();
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath);
		if (!stream.good())
		{
			HZ_CORE_ERROR("Could not open project file: {0}", filepath.string());
			return false;
		}

		YAML::Node data;
		try
		{
			data = YAML::Load(stream);
		}
		catch (const YAML::Exception& e)
		{
			HZ_CORE_ERROR("Failed to parse project YAML: {0}", e.what());
			return false;
		}

		auto root = data["Project"];
		if (!root)
		{
			HZ_CORE_ERROR("Project file missing 'Project' root node: {0}", filepath.string());
			return false;
		}

		auto& config = m_Project->GetConfig();
		config.Name = root["Name"].as<std::string>(config.Name);
		config.AssetDirectory = root["AssetDirectory"].as<std::string>(config.AssetDirectory);
		config.ScriptModulePath = root["ScriptModulePath"].as<std::string>(config.ScriptModulePath);
		config.StartScene = root["StartScene"].as<std::string>(config.StartScene);
		config.ProjectFilePath = std::filesystem::absolute(filepath);
		config.ProjectDirectory = config.ProjectFilePath.parent_path();

		return true;
	}

	bool ProjectSerializer::SerializeRuntime(const std::filesystem::path& filepath)
	{
		const auto& config = m_Project->GetConfig();
		const auto& metadata = m_Project->GetAssetManager()->GetMetadata(config.StartScene);
		if (!metadata.IsValid())
		{
			HZ_CORE_ERROR("Cannot serialize runtime project data — start scene not registered: {}", config.StartScene);
			return false;
		}

		ProjectRuntimeInfo runtimeInfo;
		runtimeInfo.StartScene = metadata.Handle;
		m_Project->GetConfig().StartSceneHandle = metadata.Handle;

		FileStreamWriter stream(filepath);
		if (!stream)
			return false;

		stream.WriteRaw(runtimeInfo);
		return stream.IsStreamGood();
	}

	bool ProjectSerializer::DeserializeRuntime(const std::filesystem::path& filepath)
	{
		FileStreamReader stream(filepath);
		if (!stream)
		{
			HZ_CORE_ERROR("Could not open runtime project file: {}", filepath.string());
			return false;
		}

		ProjectRuntimeInfo runtimeInfo;
		stream.ReadRaw(runtimeInfo);

		if (std::memcmp(runtimeInfo.Header.HEADER, "HDAT", 4) != 0)
		{
			HZ_CORE_ERROR("Invalid runtime project header in {}", filepath.string());
			return false;
		}

		if (runtimeInfo.Header.Version != ProjectRuntimeInfo{}.Header.Version)
		{
			HZ_CORE_ERROR("Unsupported runtime project version {}", runtimeInfo.Header.Version);
			return false;
		}

		m_Project->GetConfig().StartSceneHandle = runtimeInfo.StartScene;
		return true;
	}

}
