#include "hzpch.h"
#include "AssetSerializer.h"
#include "AssetManager/EditorAssetManager.h"
#include "TextureAsset.h"
#include "MeshSource.h"
#include "StaticMesh.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Scene/Prefab.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"

#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<std::vector<uint32_t>>
	{
		static Node encode(const std::vector<uint32_t>& value)
		{
			Node node;
			for (uint32_t element : value)
				node.push_back(element);
			return node;
		}

		static bool decode(const Node& node, std::vector<uint32_t>& result)
		{
			if (!node.IsSequence())
				return false;

			result.resize(node.size());
			for (size_t i = 0; i < node.size(); i++)
				result[i] = node[i].as<uint32_t>();

			return true;
		}
	};

}

namespace Hazel {

	YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<uint32_t>& value)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq;
		for (uint32_t element : value)
			out << element;
		out << YAML::EndSeq;
		return out;
	}

	namespace {

		Buffer ReadFileBytes(const std::filesystem::path& path)
		{
			std::ifstream stream(path, std::ios::binary);
			stream.seekg(0, std::ios::end);
			const auto size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			Buffer buffer;
			buffer.Allocate((uint64_t)size);
			if (size > 0)
				stream.read((char*)buffer.Data, size);
			return buffer;
		}

	}

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		(void)metadata;
		(void)asset;
	}

	bool TextureSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		const auto path = Project::GetActive()->GetAssetPath(metadata.FilePath).string();
		auto textureAsset = CreateRef<TextureAsset>();
		textureAsset->Handle = metadata.Handle;
		textureAsset->Texture = Texture2D::Create(path);
		asset = textureAsset;
		return textureAsset->Texture != nullptr;
	}

	bool TextureSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		const auto& metadata = Project::GetActiveAssetManager()->GetMetadata(handle);
		Buffer fileData = ReadFileBytes(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));

		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteBuffer(fileData);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		fileData.Release();
		return true;
	}

	Ref<Asset> TextureSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		Buffer buffer;
		stream.ReadBuffer(buffer, assetInfo.PackedSize);

		auto textureAsset = CreateRef<TextureAsset>();
		textureAsset->Texture = Texture2D::CreateFromMemory(buffer.Data, (size_t)buffer.Size);
		buffer.Release();

		if (!textureAsset->Texture)
			return nullptr;

		return textureAsset;
	}

	void PrefabSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Prefab> prefab = std::dynamic_pointer_cast<Prefab>(asset);
		if (!prefab)
			return;

		std::string yamlString = SerializeToYAML(prefab);
		std::ofstream fout(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));
		fout << yamlString;
	}

	bool PrefabSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));
		if (!stream.is_open())
			return false;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<Prefab> prefab = CreateRef<Prefab>();
		if (!DeserializeFromYAML(strStream.str(), prefab))
			return false;

		asset = prefab;
		asset->Handle = metadata.Handle;
		prefab->Handle = metadata.Handle;
		return true;
	}

	bool PrefabSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
		if (!prefab)
			return false;

		std::string yamlString = SerializeToYAML(prefab);
		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> PrefabSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<Prefab> prefab = CreateRef<Prefab>();
		if (!DeserializeFromYAML(yamlString, prefab))
			return nullptr;

		return prefab;
	}

	std::string PrefabSerializer::SerializeToYAML(const Ref<Prefab>& prefab) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab";
		out << YAML::Value << YAML::BeginSeq;

		prefab->m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID)
			{
				Entity entity = { entityID, prefab->m_Scene.get() };
				if (!entity || !entity.HasComponent<IDComponent>())
					return;

				SceneSerializer::SerializeEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;
		return std::string(out.c_str());
	}

	bool PrefabSerializer::DeserializeFromYAML(const std::string& yamlString, const Ref<Prefab>& prefab) const
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["Prefab"])
			return false;

		prefab->m_Scene = Scene::CreateEmpty();
		YAML::Node prefabNode = data["Prefab"];
		SceneSerializer::DeserializeEntities(prefabNode, prefab->m_Scene);

		prefab->m_Scene->m_Registry.view<PrefabComponent>().each([&](auto entityID, auto&)
			{
				if (!prefab->m_Entity)
					prefab->m_Entity = { entityID, prefab->m_Scene.get() };
			});

		return (bool)prefab->m_Entity;
	}

	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		(void)asset;
		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);
		serializer.Serialize(Project::GetActiveAssetManager()->GetFileSystemPath(metadata).string());
	}

	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		(void)metadata;
		(void)asset;
		return true;
	}

	bool SceneAssetSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<Scene> scene = CreateRef<Scene>();
		const auto& metadata = Project::GetActiveAssetManager()->GetMetadata(handle);
		SceneSerializer serializer(scene);
		if (!serializer.Deserialize(Project::GetActiveAssetManager()->GetFileSystemPath(metadata).string()))
			return false;

		return serializer.SerializeToAssetPack(stream, outInfo);
	}

	Ref<Asset> SceneAssetSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		(void)stream;
		(void)assetInfo;
		return nullptr;
	}

	Ref<Scene> SceneAssetSerializer::DeserializeSceneFromAssetPack(FileStreamReader& stream, const AssetPackFile::SceneInfo& sceneInfo) const
	{
		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);
		if (serializer.DeserializeFromAssetPack(stream, sceneInfo))
			return scene;
		return nullptr;
	}

	void MeshSourceSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<MeshSource> meshSource = std::dynamic_pointer_cast<MeshSource>(asset);
		if (!meshSource)
			return;

		std::ofstream fout(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));
		fout << SerializeToYAML(meshSource);
	}

	bool MeshSourceSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));
		if (!stream.is_open())
			return false;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<MeshSource> meshSource;
		if (!DeserializeFromYAML(strStream.str(), meshSource))
			return false;

		asset = meshSource;
		asset->Handle = metadata.Handle;
		return true;
	}

	bool MeshSourceSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<MeshSource> meshSource = AssetManager::GetAsset<MeshSource>(handle);
		if (!meshSource)
			return false;

		std::string yamlString = SerializeToYAML(meshSource);
		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> MeshSourceSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<MeshSource> meshSource;
		if (!DeserializeFromYAML(yamlString, meshSource))
			return nullptr;

		return meshSource;
	}

	std::string MeshSourceSerializer::SerializeToYAML(const Ref<MeshSource>& meshSource) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "MeshSource";
		out << YAML::BeginMap;
		out << YAML::Key << "Positions" << YAML::Value << YAML::BeginSeq;
		for (const auto& position : meshSource->GetPositions())
		{
			out << YAML::Flow << YAML::BeginSeq << position.x << position.y << position.z << YAML::EndSeq;
		}
		out << YAML::EndSeq;
		out << YAML::Key << "Indices" << YAML::Value << meshSource->GetIndices();
		out << YAML::EndMap;
		out << YAML::EndMap;
		return std::string(out.c_str());
	}

	bool MeshSourceSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<MeshSource>& target) const
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["MeshSource"])
			return false;

		YAML::Node rootNode = data["MeshSource"];
		if (!rootNode["Positions"] || !rootNode["Indices"])
			return false;

		std::vector<glm::vec3> positions;
		for (const auto& positionNode : rootNode["Positions"])
			positions.push_back(positionNode.as<glm::vec3>());

		std::vector<uint32_t> indices = rootNode["Indices"].as<std::vector<uint32_t>>();
		if (positions.empty() || indices.empty())
			return false;

		target = CreateRef<MeshSource>(positions, indices);
		return true;
	}

	void StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<StaticMesh> staticMesh = std::dynamic_pointer_cast<StaticMesh>(asset);
		if (!staticMesh)
			return;

		std::ofstream fout(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));
		fout << SerializeToYAML(staticMesh);
	}

	bool StaticMeshSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(Project::GetActiveAssetManager()->GetFileSystemPath(metadata));
		if (!stream.is_open())
			return false;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<StaticMesh> staticMesh;
		if (!DeserializeFromYAML(strStream.str(), staticMesh))
			return false;

		if (!AssetManager::GetAsset<MeshSource>(staticMesh->GetMeshSource()))
			return false;

		asset = staticMesh;
		asset->Handle = metadata.Handle;
		staticMesh->Handle = metadata.Handle;
		return true;
	}

	bool StaticMeshSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(handle);
		if (!staticMesh)
			return false;

		std::string yamlString = SerializeToYAML(staticMesh);
		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> StaticMeshSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<StaticMesh> staticMesh;
		if (!DeserializeFromYAML(yamlString, staticMesh))
			return nullptr;

		return staticMesh;
	}

	std::string StaticMeshSerializer::SerializeToYAML(const Ref<StaticMesh>& staticMesh) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Mesh";
		out << YAML::BeginMap;
		out << YAML::Key << "MeshSource" << YAML::Value << (uint64_t)staticMesh->GetMeshSource();
		out << YAML::Key << "SubmeshIndices" << YAML::Value << std::vector<uint32_t>();
		out << YAML::EndMap;
		out << YAML::EndMap;
		return std::string(out.c_str());
	}

	bool StaticMeshSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<StaticMesh>& target) const
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["Mesh"])
			return false;

		YAML::Node rootNode = data["Mesh"];
		if (!rootNode["MeshSource"])
			return false;

		AssetHandle meshSource = rootNode["MeshSource"].as<uint64_t>();
		target = StaticMesh::Create(meshSource);
		return true;
	}

}
