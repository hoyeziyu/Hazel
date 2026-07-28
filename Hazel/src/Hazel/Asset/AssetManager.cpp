#include "hzpch.h"
#include "AssetManager.h"

#include "AssetManager/EditorAssetManager.h"
#include "AssetManager/RuntimeAssetManager.h"
#include "Hazel/Project/Project.h"

namespace Hazel {

	namespace {

		Ref<EditorAssetManager> GetEditorManager()
		{
			auto project = Project::GetActive();
			return project ? project->GetAssetManager() : nullptr;
		}

		Ref<RuntimeAssetManager> GetRuntimeManager()
		{
			return Project::GetRuntimeAssetManager();
		}

	}

	bool AssetManager::IsAssetHandleValid(AssetHandle handle)
	{
		if (Project::IsRuntimeActive())
		{
			auto manager = GetRuntimeManager();
			return manager && manager->IsAssetHandleValid(handle);
		}

		auto manager = GetEditorManager();
		return manager && manager->IsAssetHandleValid(handle);
	}

	bool AssetManager::IsAssetValid(AssetHandle handle)
	{
		if (Project::IsRuntimeActive())
			return GetAsset(handle) != nullptr;

		auto manager = GetEditorManager();
		return manager && manager->IsAssetValid(handle);
	}

	AssetType AssetManager::GetAssetType(AssetHandle handle)
	{
		if (Project::IsRuntimeActive())
		{
			auto manager = GetRuntimeManager();
			return manager ? manager->GetAssetType(handle) : AssetType::None;
		}

		auto manager = GetEditorManager();
		return manager ? manager->GetAssetType(handle) : AssetType::None;
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle handle)
	{
		static AssetMetadata s_Null;
		if (Project::IsRuntimeActive())
			return s_Null;

		auto manager = GetEditorManager();
		return manager ? manager->GetMetadata(handle) : s_Null;
	}

	std::filesystem::path AssetManager::GetFileSystemPath(AssetHandle handle)
	{
		if (Project::IsRuntimeActive())
			return {};

		auto manager = GetEditorManager();
		return manager ? manager->GetFileSystemPath(handle) : std::filesystem::path{};
	}

	AssetHandle AssetManager::ImportAsset(const std::filesystem::path& filepath)
	{
		if (Project::IsRuntimeActive())
			return AssetHandle(0);

		auto manager = GetEditorManager();
		return manager ? manager->ImportAsset(filepath) : AssetHandle(0);
	}

	AssetHandle AssetManager::CreateNewAsset(const std::filesystem::path& relativePath)
	{
		if (Project::IsRuntimeActive())
			return AssetHandle(0);

		auto manager = GetEditorManager();
		return manager ? manager->CreateNewAsset(relativePath) : AssetHandle(0);
	}

	void AssetManager::SetLoadedAsset(AssetHandle handle, const Ref<Asset>& asset)
	{
		if (Project::IsRuntimeActive())
			return;

		auto manager = GetEditorManager();
		if (manager)
			manager->SetLoadedAsset(handle, asset);
	}

	Ref<Asset> AssetManager::GetAsset(AssetHandle handle)
	{
		if (Project::IsRuntimeActive())
		{
			auto manager = GetRuntimeManager();
			return manager ? manager->GetAsset(handle) : nullptr;
		}

		auto manager = GetEditorManager();
		return manager ? manager->GetAsset(handle) : nullptr;
	}

	std::unordered_set<AssetHandle> AssetManager::GetAllAssetsWithType(AssetType type)
	{
		if (Project::IsRuntimeActive())
			return {};

		auto manager = GetEditorManager();
		return manager ? manager->GetAllAssetsWithType(type) : std::unordered_set<AssetHandle>{};
	}

	size_t AssetManager::GetRegistryEntryCount()
	{
		if (Project::IsRuntimeActive())
			return 0;

		auto manager = GetEditorManager();
		return manager ? manager->GetRegistryEntryCount() : 0;
	}

}
