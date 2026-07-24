#include "hzpch.h"
#include "AssetImporter.h"

#include "AssetSerializer.h"
#include "AssetManager/EditorAssetManager.h"
#include "TextureAsset.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Scene/Prefab.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/SceneSerializer.h"

namespace Hazel {

	namespace {

		Scope<TextureSerializer> s_TextureSerializer;
		Scope<PrefabSerializer> s_PrefabSerializer;
		Scope<SceneAssetSerializer> s_SceneAssetSerializer;
		Scope<MeshSourceSerializer> s_MeshSourceSerializer;
		Scope<StaticMeshSerializer> s_StaticMeshSerializer;
		Scope<MaterialAssetSerializer> s_MaterialAssetSerializer;

	}

	void AssetImporter::Init()
	{
		if (!s_TextureSerializer)
		{
			s_TextureSerializer = CreateScope<TextureSerializer>();
			s_PrefabSerializer = CreateScope<PrefabSerializer>();
			s_SceneAssetSerializer = CreateScope<SceneAssetSerializer>();
			s_MeshSourceSerializer = CreateScope<MeshSourceSerializer>();
			s_StaticMeshSerializer = CreateScope<StaticMeshSerializer>();
			s_MaterialAssetSerializer = CreateScope<MaterialAssetSerializer>();
		}
	}

	AssetSerializer* AssetImporter::GetSerializer(AssetType type)
	{
		Init();
		switch (type)
		{
		case AssetType::Texture:    return s_TextureSerializer.get();
		case AssetType::Prefab:     return s_PrefabSerializer.get();
		case AssetType::Scene:      return s_SceneAssetSerializer.get();
		case AssetType::MeshSource: return s_MeshSourceSerializer.get();
		case AssetType::StaticMesh: return s_StaticMeshSerializer.get();
		case AssetType::Material: return s_MaterialAssetSerializer.get();
		default: return nullptr;
		}
	}

	void AssetImporter::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
	{
		if (auto* serializer = GetSerializer(metadata.Type))
			serializer->Serialize(metadata, asset);
	}

	void AssetImporter::Serialize(const Ref<Asset>& asset)
	{
		if (!asset || (uint64_t)asset->Handle == 0)
			return;

		const auto& metadata = Project::GetActiveAssetManager()->GetMetadata(asset->Handle);
		Serialize(metadata, asset);
	}

	bool AssetImporter::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset)
	{
		if (!metadata.IsValid())
			return false;

		if (auto* serializer = GetSerializer(metadata.Type))
			return serializer->TryLoadData(metadata, asset);

		switch (metadata.Type)
		{
		case AssetType::Scene:
			return true;
		default:
			return false;
		}
	}

	bool AssetImporter::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo)
	{
		AssetType type = Project::GetActiveAssetManager()->GetAssetType(handle);
		if (auto* serializer = GetSerializer(type))
			return serializer->SerializeToAssetPack(handle, stream, outInfo);
		return false;
	}

	Ref<Asset> AssetImporter::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo)
	{
		if (auto* serializer = GetSerializer((AssetType)assetInfo.Type))
			return serializer->DeserializeFromAssetPack(stream, assetInfo);
		return nullptr;
	}

	Ref<Scene> AssetImporter::DeserializeSceneFromAssetPack(FileStreamReader& stream, const AssetPackFile::SceneInfo& sceneInfo)
	{
		Init();
		return s_SceneAssetSerializer->DeserializeSceneFromAssetPack(stream, sceneInfo);
	}

}
