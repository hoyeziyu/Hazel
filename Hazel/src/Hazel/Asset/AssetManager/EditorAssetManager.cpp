#include "hzpch.h"
#include "EditorAssetManager.h"

#include "Hazel/Asset/AssetExtensions.h"
#include "Hazel/Asset/AssetImporter.h"
#include "Hazel/Project/Project.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

#include <yaml-cpp/yaml.h>

namespace Hazel {

	static AssetMetadata s_NullMetadata;

	EditorAssetManager::EditorAssetManager(bool scanAssets)
	{
		LoadAssetRegistry();
		if (scanAssets)
			ScanAndRegisterAssets();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		Shutdown();
	}

	void EditorAssetManager::Shutdown()
	{
		WriteRegistryToFile();
		m_LoadedAssets.clear();
	}

	bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return (uint64_t)handle != 0 && m_AssetRegistry.Contains(handle);
	}

	bool EditorAssetManager::IsAssetValid(AssetHandle handle)
	{
		return GetAsset(handle) != nullptr;
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
	{
		if (!IsAssetHandleValid(handle))
			return AssetType::None;
		return m_AssetRegistry.Get(handle).Type;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
	{
		if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry.Get(handle);
		return s_NullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const std::filesystem::path& filepath) const
	{
		const auto relativePath = GetRelativePath(filepath);
		for (const auto& [handle, metadata] : m_AssetRegistry.GetAssets())
		{
			(void)handle;
			if (metadata.FilePath == relativePath)
				return metadata;
		}
		return s_NullMetadata;
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
	{
		const auto relativePath = GetRelativePath(filepath);
		if (relativePath.filename() == "AssetRegistry.hzr")
			return 0;

		if (const auto& existing = GetMetadata(relativePath); existing.IsValid())
			return existing.Handle;

		const AssetType type = GetAssetTypeFromPath(relativePath);
		if (type == AssetType::None)
			return 0;

		AssetMetadata metadata;
		metadata.Handle = AssetHandle();
		metadata.FilePath = relativePath;
		metadata.Type = type;
		m_AssetRegistry.Set(metadata);
		return metadata.Handle;
	}

	AssetHandle EditorAssetManager::CreateNewAsset(const std::filesystem::path& relativePath)
	{
		const auto fullPath = Project::GetActive()->GetAssetPath(relativePath);
		std::error_code ec;
		std::filesystem::create_directories(fullPath.parent_path(), ec);
		{
			std::ofstream stream(fullPath);
			(void)stream;
		}
		return ImportAsset(fullPath);
	}

	void EditorAssetManager::SetLoadedAsset(AssetHandle handle, const Ref<Asset>& asset)
	{
		if ((uint64_t)handle != 0 && asset)
			m_LoadedAssets[handle] = asset;
	}

	void EditorAssetManager::ScanAndRegisterAssets()
	{
		auto project = Project::GetActive();
		if (!project)
			return;

		ProcessDirectory(project->GetAssetDirectory());
		WriteRegistryToFile();
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(const AssetMetadata& metadata) const
	{
		return Project::GetActive()->GetAssetPath(metadata.FilePath);
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(AssetHandle handle) const
	{
		return GetFileSystemPath(GetMetadata(handle));
	}

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		if (!IsAssetHandleValid(handle))
			return nullptr;

		if (auto it = m_LoadedAssets.find(handle); it != m_LoadedAssets.end())
			return it->second;

		auto metadata = GetMetadata(handle);
		if (metadata.Type == AssetType::Scene)
			return nullptr;

		Ref<Asset> asset;
		if (!AssetImporter::TryLoadData(metadata, asset) || !asset)
			return nullptr;

		metadata.IsDataLoaded = true;
		m_AssetRegistry.Set(metadata);
		m_LoadedAssets[handle] = asset;
		return asset;
	}

	std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type) const
	{
		std::unordered_set<AssetHandle> result;
		for (const auto& [handle, metadata] : m_AssetRegistry.GetAssets())
		{
			if (metadata.Type == type)
				result.insert(handle);
		}
		return result;
	}

	void EditorAssetManager::LoadAssetRegistry()
	{
		const auto registryPath = Project::GetActiveAssetRegistryPath();
		std::error_code ec;
		if (!std::filesystem::exists(registryPath, ec))
			return;

		std::ifstream stream(registryPath);
		if (!stream)
			return;

		std::stringstream buffer;
		buffer << stream.rdbuf();

		YAML::Node data = YAML::Load(buffer.str());
		auto assets = data["Assets"];
		if (!assets || !assets.IsSequence())
			return;

		for (size_t i = 0; i < assets.size(); ++i)
		{
			const YAML::Node& entry = assets[i];

			AssetMetadata metadata;
			metadata.Handle = entry["Handle"].as<uint64_t>();
			metadata.FilePath = entry["FilePath"].as<std::string>();
			metadata.Type = AssetTypeFromString(entry["Type"].as<std::string>());

			if (!metadata.IsValid())
				continue;

			if (metadata.Type != GetAssetTypeFromPath(metadata.FilePath))
				metadata.Type = GetAssetTypeFromPath(metadata.FilePath);

			if (!std::filesystem::exists(GetFileSystemPath(metadata), ec))
			{
				HZ_CORE_WARN("Asset registry entry missing on disk: {}", metadata.FilePath.string());
				continue;
			}

			m_AssetRegistry.Set(metadata);
		}

		HZ_CORE_INFO("Loaded {} asset registry entries", m_AssetRegistry.Count());
	}

	void EditorAssetManager::WriteRegistryToFile()
	{
		struct Entry
		{
			std::string FilePath;
			AssetType Type;
		};

		std::map<UUID, Entry> sorted;
		std::error_code ec;
		for (const auto& [handle, metadata] : m_AssetRegistry.GetAssets())
		{
			(void)handle;
			if (!std::filesystem::exists(GetFileSystemPath(metadata), ec))
				continue;

			std::string path = metadata.FilePath.generic_string();
			sorted[metadata.Handle] = { path, metadata.Type };
		}

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;
		for (const auto& [handle, entry] : sorted)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << (uint64_t)handle;
			out << YAML::Key << "FilePath" << YAML::Value << entry.FilePath;
			out << YAML::Key << "Type" << YAML::Value << std::string(AssetTypeToString(entry.Type));
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		const auto registryPath = Project::GetActiveAssetRegistryPath();
		std::filesystem::create_directories(registryPath.parent_path(), ec);
		std::ofstream fout(registryPath);
		fout << out.c_str();
	}

	void EditorAssetManager::ProcessDirectory(const std::filesystem::path& directoryPath)
	{
		std::error_code ec;
		if (!std::filesystem::exists(directoryPath, ec))
			return;

		for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath, ec))
		{
			if (ec)
				break;
			if (entry.is_directory())
				continue;
			ImportAsset(entry.path());
		}
	}

	std::filesystem::path EditorAssetManager::GetRelativePath(const std::filesystem::path& filepath) const
	{
		const auto assetRoot = Project::GetActive()->GetAssetDirectory();
		std::error_code ec;
		auto relative = std::filesystem::relative(filepath, assetRoot, ec);
		if (!ec && !relative.empty())
			return relative.lexically_normal();
		return filepath.lexically_normal();
	}

}
