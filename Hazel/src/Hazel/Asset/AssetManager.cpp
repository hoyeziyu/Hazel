#include "hzpch.h"
#include "AssetManager.h"

#include "AssetManager/EditorAssetManager.h"
#include "Hazel/Project/Project.h"

namespace Hazel {

	static Ref<EditorAssetManager> GetManager()
	{
		auto project = Project::GetActive();
		return project ? project->GetAssetManager() : nullptr;
	}

	bool AssetManager::IsAssetHandleValid(AssetHandle handle)
	{
		auto manager = GetManager();
		return manager && manager->IsAssetHandleValid(handle);
	}

	bool AssetManager::IsAssetValid(AssetHandle handle)
	{
		auto manager = GetManager();
		return manager && manager->IsAssetValid(handle);
	}

	AssetType AssetManager::GetAssetType(AssetHandle handle)
	{
		auto manager = GetManager();
		return manager ? manager->GetAssetType(handle) : AssetType::None;
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle handle)
	{
		static AssetMetadata s_Null;
		auto manager = GetManager();
		return manager ? manager->GetMetadata(handle) : s_Null;
	}

	std::filesystem::path AssetManager::GetFileSystemPath(AssetHandle handle)
	{
		auto manager = GetManager();
		return manager ? manager->GetFileSystemPath(handle) : std::filesystem::path{};
	}

	AssetHandle AssetManager::ImportAsset(const std::filesystem::path& filepath)
	{
		auto manager = GetManager();
		return manager ? manager->ImportAsset(filepath) : AssetHandle(0);
	}

	AssetHandle AssetManager::CreateNewAsset(const std::filesystem::path& relativePath)
	{
		auto manager = GetManager();
		return manager ? manager->CreateNewAsset(relativePath) : AssetHandle(0);
	}

	void AssetManager::SetLoadedAsset(AssetHandle handle, const Ref<Asset>& asset)
	{
		auto manager = GetManager();
		if (manager)
			manager->SetLoadedAsset(handle, asset);
	}

	Ref<Asset> AssetManager::GetAsset(AssetHandle handle)
	{
		auto manager = GetManager();
		return manager ? manager->GetAsset(handle) : nullptr;
	}

	std::unordered_set<AssetHandle> AssetManager::GetAllAssetsWithType(AssetType type)
	{
		auto manager = GetManager();
		return manager ? manager->GetAllAssetsWithType(type) : std::unordered_set<AssetHandle>{};
	}

	size_t AssetManager::GetRegistryEntryCount()
	{
		auto manager = GetManager();
		return manager ? manager->GetRegistryEntryCount() : 0;
	}

}
