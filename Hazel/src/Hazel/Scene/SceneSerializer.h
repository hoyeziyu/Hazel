#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Serialization/FileStream.h"
#include "Hazel/Serialization/AssetPackFile.h"
#include "Hazel/Asset/AssetSerializer.h"
#include "Scene.h"

namespace YAML {
	class Emitter;
	class Node;
}

namespace Hazel {
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
		bool DeserializeFromYAML(const std::string& yamlString);

		bool SerializeToAssetPack(FileStreamWriter& stream, AssetSerializationInfo& outInfo);
		bool DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::SceneInfo& sceneInfo);

		static void SerializeEntity(YAML::Emitter& out, Entity entity);
		static void DeserializeEntities(YAML::Node& entitiesNode, const Ref<Scene>& scene);

	private:
		void SerializeToYAML(YAML::Emitter& out);

	private:
		Ref<Scene> m_Scene;
	};

}
