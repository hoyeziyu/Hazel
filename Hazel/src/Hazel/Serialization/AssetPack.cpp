#include "hzpch.h"
#include "AssetPack.h"

#include "Hazel/Asset/AssetManager/EditorAssetManager.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/AssetImporter.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Scene/Prefab.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/SceneSerializer.h"

#include <chrono>

namespace Hazel {

	AssetPack::AssetPack(const std::filesystem::path& path)
		: m_Path(path)
	{
	}

	Ref<Scene> AssetPack::LoadScene(AssetHandle sceneHandle)
	{
		auto it = m_File.Index.Scenes.find(sceneHandle);
		if (it == m_File.Index.Scenes.end())
			return nullptr;

		FileStreamReader stream(m_Path);
		return AssetImporter::DeserializeSceneFromAssetPack(stream, it->second);
	}

	Ref<Asset> AssetPack::LoadAsset(AssetHandle sceneHandle, AssetHandle assetHandle)
	{
		const AssetPackFile::AssetInfo* assetInfo = nullptr;

		if ((uint64_t)sceneHandle != 0)
		{
			auto it = m_File.Index.Scenes.find(sceneHandle);
			if (it != m_File.Index.Scenes.end())
			{
				auto assetIt = it->second.Assets.find(assetHandle);
				if (assetIt != it->second.Assets.end())
					assetInfo = &assetIt->second;
			}
		}

		if (!assetInfo)
		{
			for (const auto& [handle, sceneInfo] : m_File.Index.Scenes)
			{
				(void)handle;
				auto assetIt = sceneInfo.Assets.find(assetHandle);
				if (assetIt != sceneInfo.Assets.end())
				{
					assetInfo = &assetIt->second;
					break;
				}
			}
		}

		if (!assetInfo)
			return nullptr;

		FileStreamReader stream(m_Path);
		Ref<Asset> asset = AssetImporter::DeserializeFromAssetPack(stream, *assetInfo);
		if (asset)
			asset->Handle = assetHandle;
		return asset;
	}

	bool AssetPack::IsAssetHandleValid(AssetHandle assetHandle) const
	{
		return m_AssetHandleIndex.find(assetHandle) != m_AssetHandleIndex.end();
	}

	AssetType AssetPack::GetAssetType(AssetHandle sceneHandle, AssetHandle assetHandle) const
	{
		if ((uint64_t)sceneHandle != 0)
		{
			auto it = m_File.Index.Scenes.find(sceneHandle);
			if (it != m_File.Index.Scenes.end())
			{
				auto assetIt = it->second.Assets.find(assetHandle);
				if (assetIt != it->second.Assets.end())
					return (AssetType)assetIt->second.Type;
			}
		}

		for (const auto& [handle, sceneInfo] : m_File.Index.Scenes)
		{
			(void)handle;
			auto assetIt = sceneInfo.Assets.find(assetHandle);
			if (assetIt != sceneInfo.Assets.end())
				return (AssetType)assetIt->second.Type;
		}

		return AssetType::None;
	}

	bool AssetPack::CreateFromActiveProject(const std::filesystem::path& outputPath, std::atomic<float>& progress)
	{
		auto project = Project::GetActive();
		if (!project)
			return false;

		AssetImporter::Init();
		progress = 0.0f;

		AssetPackFile assetPackFile;
		const auto now = std::chrono::system_clock::now();
		const auto time = std::chrono::system_clock::to_time_t(now);
		std::tm localTime{};
		localtime_s(&localTime, &time);
		assetPackFile.Header.BuildVersion =
			(uint64_t)(localTime.tm_year + 1900) * 10000000000ULL +
			(uint64_t)(localTime.tm_mon + 1) * 100000000ULL +
			(uint64_t)localTime.tm_mday * 1000000ULL +
			(uint64_t)localTime.tm_hour * 10000ULL +
			(uint64_t)localTime.tm_min * 100ULL +
			(uint64_t)localTime.tm_sec;

		std::unordered_set<AssetHandle> sceneHandles = AssetManager::GetAllAssetsWithType(AssetType::Scene);
		if (sceneHandles.empty())
		{
			HZ_CORE_WARN("No scenes registered for AssetPack build");
			return false;
		}

		float progressIncrement = 0.15f / (float)sceneHandles.size();

		for (AssetHandle sceneHandle : sceneHandles)
		{
			const auto& metadata = project->GetAssetManager()->GetMetadata(sceneHandle);
			Ref<Scene> scene = CreateRef<Scene>();
			SceneSerializer serializer(scene);
			const auto scenePath = project->GetAssetManager()->GetFileSystemPath(metadata);

			if (!serializer.Deserialize(scenePath.string()))
			{
				HZ_CORE_WARN("Failed to deserialize scene for AssetPack: {}", metadata.FilePath.string());
				continue;
			}

			std::unordered_set<AssetHandle> sceneAssetList = scene->GetAssetList();
			for (AssetHandle assetHandle : sceneAssetList)
			{
				if (AssetManager::GetAssetType(assetHandle) == AssetType::Prefab)
				{
					if (auto prefab = AssetManager::GetAsset<Prefab>(assetHandle))
					{
						auto childAssets = prefab->GetAssetList(true);
						sceneAssetList.insert(childAssets.begin(), childAssets.end());
					}
				}
			}

			AssetPackFile::SceneInfo& sceneInfo = assetPackFile.Index.Scenes[sceneHandle];
			for (AssetHandle assetHandle : sceneAssetList)
			{
				AssetPackFile::AssetInfo& assetInfo = sceneInfo.Assets[assetHandle];
				assetInfo.Type = (uint16_t)AssetManager::GetAssetType(assetHandle);
			}

			progress.store(progress.load() + progressIncrement);
		}

		if (assetPackFile.Index.Scenes.empty())
			return false;

		Buffer emptyAppBinary;
		AssetPackSerializer::Serialize(outputPath, assetPackFile, emptyAppBinary, progress);
		return true;
	}

	Ref<AssetPack> AssetPack::Load(const std::filesystem::path& path)
	{
		Ref<AssetPack> assetPack = CreateRef<AssetPack>(path);
		if (!AssetPackSerializer::DeserializeIndex(path, assetPack->m_File))
			return nullptr;

		for (const auto& [sceneHandle, sceneInfo] : assetPack->m_File.Index.Scenes)
		{
			assetPack->m_AssetHandleIndex.insert(sceneHandle);
			for (const auto& [assetHandle, assetInfo] : sceneInfo.Assets)
			{
				(void)assetInfo;
				assetPack->m_AssetHandleIndex.insert(assetHandle);
			}
		}

		return assetPack;
	}

}
