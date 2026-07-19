#pragma once

#include "Hazel/Asset/AssetMetadata.h"
#include "Hazel/Serialization/AssetPack.h"

#include <unordered_map>

namespace Hazel {

	class RuntimeAssetManager
	{
	public:
		RuntimeAssetManager() = default;

		void SetAssetPack(const Ref<AssetPack>& assetPack) { m_AssetPack = assetPack; }
		void SetActiveScene(AssetHandle sceneHandle) { m_ActiveScene = sceneHandle; }

		bool IsAssetHandleValid(AssetHandle handle) const;
		AssetType GetAssetType(AssetHandle handle) const;

		Ref<Asset> GetAsset(AssetHandle handle);
		Ref<Scene> LoadScene(AssetHandle handle);

		template<typename TAsset>
		Ref<TAsset> GetAsset(AssetHandle handle)
		{
			return std::dynamic_pointer_cast<TAsset>(GetAsset(handle));
		}

	private:
		Ref<AssetPack> m_AssetPack;
		AssetHandle m_ActiveScene = 0;
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
	};

}
