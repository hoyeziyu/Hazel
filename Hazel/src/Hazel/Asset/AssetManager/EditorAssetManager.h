#pragma once

#include "Hazel/Asset/AssetMetadata.h"
#include "Hazel/Asset/AssetRegistry.h"

#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace Hazel {

	class EditorAssetManager
	{
	public:
		explicit EditorAssetManager(bool scanAssets = true);
		~EditorAssetManager();

		void Shutdown();

		bool IsAssetHandleValid(AssetHandle handle) const;
		bool IsAssetValid(AssetHandle handle);

		AssetType GetAssetType(AssetHandle handle) const;
		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		const AssetMetadata& GetMetadata(const std::filesystem::path& filepath) const;

		AssetHandle ImportAsset(const std::filesystem::path& filepath);
		AssetHandle CreateNewAsset(const std::filesystem::path& relativePath);
		void SetLoadedAsset(AssetHandle handle, const Ref<Asset>& asset);
		void ScanAndRegisterAssets();

		std::filesystem::path GetFileSystemPath(AssetHandle handle) const;
		std::filesystem::path GetFileSystemPath(const AssetMetadata& metadata) const;

		Ref<Asset> GetAsset(AssetHandle handle);
		const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() const { return m_LoadedAssets; }

		std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) const;
		size_t GetRegistryEntryCount() const { return m_AssetRegistry.Count(); }

		void WriteRegistryToFile();

	private:
		void LoadAssetRegistry();
		void ProcessDirectory(const std::filesystem::path& directoryPath);
		std::filesystem::path GetRelativePath(const std::filesystem::path& filepath) const;

	private:
		AssetRegistry m_AssetRegistry;
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
		bool m_PersistRegistry = true;
	};

}
