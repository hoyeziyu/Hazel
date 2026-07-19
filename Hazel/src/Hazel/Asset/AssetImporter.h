#pragma once

#include "Hazel/Asset/AssetMetadata.h"
#include "Hazel/Asset/AssetSerializer.h"
#include "Hazel/Serialization/FileStream.h"
#include "Hazel/Serialization/AssetPackFile.h"

namespace Hazel {

	class Scene;

	class AssetImporter
	{
	public:
		static void Init();
		static void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset);
		static void Serialize(const Ref<Asset>& asset);
		static bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset);

		static bool SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo);
		static Ref<Asset> DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo);
		static Ref<Scene> DeserializeSceneFromAssetPack(FileStreamReader& stream, const AssetPackFile::SceneInfo& sceneInfo);

	private:
		static AssetSerializer* GetSerializer(AssetType type);
	};

}
