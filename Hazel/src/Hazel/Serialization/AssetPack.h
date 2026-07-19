#pragma once

#include "AssetPackFile.h"
#include "AssetPackSerializer.h"

#include "Hazel/Asset/Asset.h"
#include "Hazel/Asset/AssetTypes.h"
#include "Hazel/Core/Core.h"

#include <atomic>
#include <filesystem>
#include <unordered_set>

namespace Hazel {

	class Scene;

	class AssetPack
	{
	public:
		AssetPack() = default;
		AssetPack(const std::filesystem::path& path);

		Ref<Scene> LoadScene(AssetHandle sceneHandle);
		Ref<Asset> LoadAsset(AssetHandle sceneHandle, AssetHandle assetHandle);

		bool IsAssetHandleValid(AssetHandle assetHandle) const;
		AssetType GetAssetType(AssetHandle sceneHandle, AssetHandle assetHandle) const;

		const AssetPackFile& GetIndex() const { return m_File; }
		const std::filesystem::path& GetPath() const { return m_Path; }

		static bool CreateFromActiveProject(const std::filesystem::path& outputPath, std::atomic<float>& progress);
		static Ref<AssetPack> Load(const std::filesystem::path& path);

	private:
		std::filesystem::path m_Path;
		AssetPackFile m_File;
		std::unordered_set<AssetHandle> m_AssetHandleIndex;
	};

}
