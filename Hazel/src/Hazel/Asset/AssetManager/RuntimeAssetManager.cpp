#include "hzpch.h"
#include "RuntimeAssetManager.h"

#include "Hazel/Asset/AssetImporter.h"
#include "Hazel/Scene/Scene.h"

namespace Hazel {

	bool RuntimeAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return m_AssetPack && m_AssetPack->IsAssetHandleValid(handle);
	}

	AssetType RuntimeAssetManager::GetAssetType(AssetHandle handle) const
	{
		if (!m_AssetPack)
			return AssetType::None;
		return m_AssetPack->GetAssetType(m_ActiveScene, handle);
	}

	Ref<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle)
	{
		if (!m_AssetPack || (uint64_t)handle == 0)
			return nullptr;

		if (auto it = m_LoadedAssets.find(handle); it != m_LoadedAssets.end())
			return it->second;

		Ref<Asset> asset = m_AssetPack->LoadAsset(m_ActiveScene, handle);
		if (asset)
		{
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
		}
		return asset;
	}

	Ref<Scene> RuntimeAssetManager::LoadScene(AssetHandle handle)
	{
		if (!m_AssetPack)
			return nullptr;

		m_ActiveScene = handle;
		m_LoadedAssets.clear();

		Ref<Scene> scene = m_AssetPack->LoadScene(handle);
		return scene;
	}

}
