#include "hzpch.h"
#include "EditorAssetManager.h"

#include "Hazel/Asset/AssetExtensions.h"
#include "Hazel/Asset/AssetImporter.h"
#include "Hazel/Project/Project.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

#include <yaml-cpp/yaml.h>

namespace Hazel {

	namespace {

		uint64_t YamlReadUint64(const YAML::Node& node, uint64_t defaultValue = 0)
		{
			if (!node || !node.IsDefined() || !node.IsScalar())
				return defaultValue;

			const std::string value = node.Scalar();
			char* end = nullptr;
			const unsigned long long parsed = std::strtoull(value.c_str(), &end, 10);
			if (end == value.c_str() || (end && *end != '\0'))
				return defaultValue;
			return (uint64_t)parsed;
		}

		std::string Trim(std::string value)
		{
			auto isSpace = [](unsigned char c) { return std::isspace(c); };
			while (!value.empty() && isSpace((unsigned char)value.front()))
				value.erase(value.begin());
			while (!value.empty() && isSpace((unsigned char)value.back()))
				value.pop_back();
			return value;
		}

		std::string Unquote(std::string value)
		{
			value = Trim(value);
			if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
				return value.substr(1, value.size() - 2);
			return value;
		}

		bool ParseRegistryLine(const std::string& line, AssetMetadata& inOut, bool& entryComplete)
		{
			const std::string trimmed = Trim(line);
			if (trimmed.rfind("- Handle:", 0) == 0)
			{
				inOut = AssetMetadata{};
				const std::string handleText = Unquote(trimmed.substr(std::string("- Handle:").size()));
				char* end = nullptr;
				const unsigned long long parsed = std::strtoull(handleText.c_str(), &end, 10);
				inOut.Handle = (end != handleText.c_str() && (!end || *end == '\0')) ? (uint64_t)parsed : 0;
				entryComplete = false;
				return true;
			}
			if (trimmed.rfind("FilePath:", 0) == 0)
			{
				inOut.FilePath = Unquote(trimmed.substr(std::string("FilePath:").size()));
				return true;
			}
			if (trimmed.rfind("Type:", 0) == 0)
			{
				inOut.Type = AssetTypeFromString(Unquote(trimmed.substr(std::string("Type:").size())));
				entryComplete = true;
				return true;
			}
			return false;
		}

	}

	static AssetMetadata s_NullMetadata;

	EditorAssetManager::EditorAssetManager(bool scanAssets)
		: m_PersistRegistry(scanAssets)
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
		if (m_PersistRegistry)
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

		std::ifstream stream(registryPath.string());
		if (!stream)
			return;

		std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

		AssetMetadata current;
		bool entryComplete = false;
		size_t start = 0;
		while (start < contents.size())
		{
			size_t end = contents.find('\n', start);
			if (end == std::string::npos)
				end = contents.size();

			std::string line = contents.substr(start, end - start);
			if (!line.empty() && line.back() == '\r')
				line.pop_back();

			start = (end == contents.size()) ? contents.size() + 1 : end + 1;

			if (line.empty())
				continue;

			if (!ParseRegistryLine(line, current, entryComplete))
				continue;

			if (!entryComplete)
				continue;

			if (!current.IsValid())
				continue;

			if (current.Type != GetAssetTypeFromPath(current.FilePath))
				current.Type = GetAssetTypeFromPath(current.FilePath);

			m_AssetRegistry.Set(current);
			entryComplete = false;
		}
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
