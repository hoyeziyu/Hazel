#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Asset/AssetMetadata.h"
#include "Hazel/Asset/TextureAsset.h"

#include <filesystem>
#include <unordered_set>

namespace Hazel {

	class AssetManager
	{
	public:
		static bool IsAssetHandleValid(AssetHandle handle);
		static bool IsAssetValid(AssetHandle handle);
		static AssetType GetAssetType(AssetHandle handle);

		static const AssetMetadata& GetMetadata(AssetHandle handle);
		static std::filesystem::path GetFileSystemPath(AssetHandle handle);

		static AssetHandle ImportAsset(const std::filesystem::path& filepath);
		static AssetHandle CreateNewAsset(const std::filesystem::path& relativePath);
		static void SetLoadedAsset(AssetHandle handle, const Ref<Asset>& asset);
		static Ref<Asset> GetAsset(AssetHandle handle);

		template<typename TAsset>
		static Ref<TAsset> GetAsset(AssetHandle handle)
		{
			return std::dynamic_pointer_cast<TAsset>(GetAsset(handle));
		}

		static std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type);
		static size_t GetRegistryEntryCount();
	};

}
