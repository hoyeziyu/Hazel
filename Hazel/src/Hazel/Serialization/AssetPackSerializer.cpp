#include "hzpch.h"
#include "AssetPackSerializer.h"

#include "Hazel/Asset/AssetImporter.h"
#include "Hazel/Core/Buffer.h"

namespace Hazel {

	static void CreateDirectoriesIfNeeded(const std::filesystem::path& path)
	{
		std::filesystem::path directory = path.parent_path();
		std::error_code ec;
		if (!std::filesystem::exists(directory, ec))
			std::filesystem::create_directories(directory, ec);
	}

	void AssetPackSerializer::Serialize(const std::filesystem::path& path, AssetPackFile& file, Buffer appBinary, std::atomic<float>& progress)
	{
		HZ_CORE_INFO("Serializing AssetPack to {}", path.string());

		CreateDirectoriesIfNeeded(path);
		FileStreamWriter serializer(path);

		serializer.WriteRaw<AssetPackFile::FileHeader>(file.Header);

		uint64_t indexPos = serializer.GetStreamPosition();
		uint64_t indexTableSize = CalculateIndexTableSize(file);
		serializer.WriteZero(indexTableSize);

		std::unordered_map<AssetHandle, AssetSerializationInfo> serializedAssets;

		float progressIncrement = file.Index.Scenes.empty() ? 0.0f : 0.8f / (float)file.Index.Scenes.size();

		file.Index.PackedAppBinaryOffset = serializer.GetStreamPosition();
		serializer.WriteBuffer(appBinary);
		file.Index.PackedAppBinarySize = serializer.GetStreamPosition() - file.Index.PackedAppBinaryOffset;
		appBinary.Release();

		for (auto& [sceneHandle, sceneInfo] : file.Index.Scenes)
		{
			AssetSerializationInfo serializationInfo;
			if (AssetImporter::SerializeToAssetPack(sceneHandle, serializer, serializationInfo))
			{
				sceneInfo.PackedOffset = serializationInfo.Offset;
				sceneInfo.PackedSize = serializationInfo.Size;
			}

			for (auto& [assetHandle, assetInfo] : sceneInfo.Assets)
			{
				if (serializedAssets.find(assetHandle) != serializedAssets.end())
				{
					serializationInfo = serializedAssets.at(assetHandle);
					assetInfo.PackedOffset = serializationInfo.Offset;
					assetInfo.PackedSize = serializationInfo.Size;
				}
				else if (AssetImporter::SerializeToAssetPack(assetHandle, serializer, serializationInfo))
				{
					assetInfo.PackedOffset = serializationInfo.Offset;
					assetInfo.PackedSize = serializationInfo.Size;
					serializedAssets[assetHandle] = serializationInfo;
				}
				else
				{
					HZ_CORE_WARN("Failed to serialize asset handle {}", (uint64_t)assetHandle);
				}
			}

			progress = progress + progressIncrement;
		}

		serializer.SetStreamPosition(indexPos);
		serializer.WriteRaw<uint64_t>(file.Index.PackedAppBinaryOffset);
		serializer.WriteRaw<uint64_t>(file.Index.PackedAppBinarySize);
		serializer.WriteRaw<uint32_t>((uint32_t)file.Index.Scenes.size());

		for (auto& [sceneHandle, sceneInfo] : file.Index.Scenes)
		{
			serializer.WriteRaw<uint64_t>((uint64_t)sceneHandle);
			serializer.WriteRaw<uint64_t>(sceneInfo.PackedOffset);
			serializer.WriteRaw<uint64_t>(sceneInfo.PackedSize);
			serializer.WriteRaw<uint16_t>(sceneInfo.Flags);
			serializer.WriteMap(sceneInfo.Assets);
		}

		progress = 1.0f;
		HZ_CORE_INFO("AssetPack written with {} scenes", file.Index.Scenes.size());
	}

	bool AssetPackSerializer::DeserializeIndex(const std::filesystem::path& path, AssetPackFile& file)
	{
		FileStreamReader stream(path);
		if (!stream.IsStreamGood())
			return false;

		stream.ReadRaw<AssetPackFile::FileHeader>(file.Header);
		if (memcmp(file.Header.HEADER, "HZAP", 4) != 0)
		{
			HZ_CORE_ERROR("Invalid AssetPack header");
			return false;
		}

		AssetPackFile current;
		if (file.Header.Version != current.Header.Version)
		{
			HZ_CORE_ERROR("AssetPack version {} is not compatible with current version {}", file.Header.Version, current.Header.Version);
			return false;
		}

		stream.ReadRaw<uint64_t>(file.Index.PackedAppBinaryOffset);
		stream.ReadRaw<uint64_t>(file.Index.PackedAppBinarySize);

		uint32_t sceneCount = 0;
		stream.ReadRaw<uint32_t>(sceneCount);
		for (uint32_t i = 0; i < sceneCount; i++)
		{
			uint64_t sceneHandle = 0;
			stream.ReadRaw<uint64_t>(sceneHandle);

			AssetPackFile::SceneInfo& sceneInfo = file.Index.Scenes[sceneHandle];
			stream.ReadRaw<uint64_t>(sceneInfo.PackedOffset);
			stream.ReadRaw<uint64_t>(sceneInfo.PackedSize);
			stream.ReadRaw<uint16_t>(sceneInfo.Flags);
			stream.ReadMap(sceneInfo.Assets);
		}

		return true;
	}

	uint64_t AssetPackSerializer::CalculateIndexTableSize(const AssetPackFile& file)
	{
		uint64_t appInfoSize = sizeof(uint64_t) * 2;
		uint64_t sceneMapSize = sizeof(uint32_t) + (sizeof(AssetHandle) + sizeof(uint64_t) * 2 + sizeof(uint16_t)) * file.Index.Scenes.size();
		uint64_t assetMapSize = 0;
		for (const auto& [sceneHandle, sceneInfo] : file.Index.Scenes)
		{
			(void)sceneHandle;
			assetMapSize += sizeof(uint32_t) + (sizeof(AssetHandle) + sizeof(AssetPackFile::AssetInfo)) * sceneInfo.Assets.size();
		}

		return appInfoSize + sceneMapSize + assetMapSize;
	}

}
