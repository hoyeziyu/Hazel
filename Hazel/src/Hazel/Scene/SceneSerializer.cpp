#include "hzpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

#include <fstream>

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
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

}

namespace Hazel {
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:m_Scene(scene)
	{
	}

	void SceneSerializer::SerializeToYAML(YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		SerializeToYAML(out);

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;

		HZ_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Entity missing IDComponent");
		out << YAML::Key << "Entity" << YAML::Value << (uint64_t)entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<PrefabComponent>())
		{
			out << YAML::Key << "PrefabComponent";
			out << YAML::BeginMap;

			auto& prefabComponent = entity.GetComponent<PrefabComponent>();
			out << YAML::Key << "Prefab" << YAML::Value << (uint64_t)prefabComponent.PrefabID;
			out << YAML::Key << "Entity" << YAML::Value << (uint64_t)prefabComponent.EntityID;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			if (spriteRendererComponent.Texture)
				out << YAML::Key << "Texture" << YAML::Value << (uint64_t)spriteRendererComponent.Texture;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;

			auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << meshRendererComponent.Color;
			out << YAML::Key << "Visible" << YAML::Value << meshRendererComponent.Visible;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<StaticMeshComponent>())
		{
			out << YAML::Key << "StaticMeshComponent";
			out << YAML::BeginMap;

			auto& staticMeshComponent = entity.GetComponent<StaticMeshComponent>();
			if (staticMeshComponent.StaticMesh)
				out << YAML::Key << "StaticMesh" << YAML::Value << (uint64_t)staticMeshComponent.StaticMesh;
			if (staticMeshComponent.Material)
				out << YAML::Key << "Material" << YAML::Value << (uint64_t)staticMeshComponent.Material;
			out << YAML::Key << "Color" << YAML::Value << staticMeshComponent.Color;
			out << YAML::Key << "Visible" << YAML::Value << staticMeshComponent.Visible;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;

			auto& light = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Radiance" << YAML::Value << YAML::Flow << YAML::BeginSeq
				<< light.Radiance.x << light.Radiance.y << light.Radiance.z << YAML::EndSeq;
			out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "RigidBody2DComponent";
			out << YAML::BeginMap;

			auto& rb = entity.GetComponent<RigidBody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << (int)rb.BodyType;
			out << YAML::Key << "FixedRotation" << YAML::Value << rb.FixedRotation;
			out << YAML::Key << "GravityScale" << YAML::Value << rb.GravityScale;
			out << YAML::Key << "LinearDamping" << YAML::Value << rb.LinearDamping;
			out << YAML::Key << "AngularDamping" << YAML::Value << rb.AngularDamping;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;

			auto& box = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << box.Offset;
			out << YAML::Key << "Size" << YAML::Value << box.Size;
			out << YAML::Key << "Density" << YAML::Value << box.Density;
			out << YAML::Key << "Friction" << YAML::Value << box.Friction;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap;

			const auto& script = entity.GetComponent<NativeScriptComponent>();
			if (!script.ClassName.empty())
				out << YAML::Key << "ClassName" << YAML::Value << script.ClassName;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::DeserializeEntities(YAML::Node& entitiesNode, const Ref<Scene>& scene)
	{
		for (auto entity : entitiesNode)
		{
			uint64_t uuid = entity["Entity"].as<uint64_t>();

			std::string name;
			auto tagComponent = entity["TagComponent"];
			if (tagComponent)
				name = tagComponent["Tag"].as<std::string>();

			Entity deserializedEntity = scene->CreateEntityWithID(uuid, name);

			auto prefabComponent = entity["PrefabComponent"];
			if (prefabComponent)
			{
				auto& pb = deserializedEntity.AddComponent<PrefabComponent>();
				pb.PrefabID = prefabComponent["Prefab"].as<uint64_t>();
				pb.EntityID = prefabComponent["Entity"].as<uint64_t>();
			}

			auto transformComponent = entity["TransformComponent"];
			if (transformComponent)
			{
				auto& tc = deserializedEntity.GetComponent<TransformComponent>();
				tc.Translation = transformComponent["Translation"].as<glm::vec3>();
				tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
				tc.Scale = transformComponent["Scale"].as<glm::vec3>();
			}

			auto cameraComponent = entity["CameraComponent"];
			if (cameraComponent)
			{
				auto& cc = deserializedEntity.AddComponent<CameraComponent>();

				YAML::Node cameraProps = cameraComponent["Camera"];
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

				cc.Primary = cameraComponent["Primary"].as<bool>();
				cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
			}

			auto spriteRendererComponent = entity["SpriteRendererComponent"];
			if (spriteRendererComponent)
			{
				auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
				src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
				if (spriteRendererComponent["Texture"])
					src.Texture = spriteRendererComponent["Texture"].as<uint64_t>();
			}

			auto meshRendererComponent = entity["MeshRendererComponent"];
			if (meshRendererComponent)
			{
				auto& src = deserializedEntity.AddComponent<MeshRendererComponent>();
				src.Color = meshRendererComponent["Color"].as<glm::vec4>();
				if (meshRendererComponent["Visible"])
					src.Visible = meshRendererComponent["Visible"].as<bool>();
			}

			auto staticMeshComponent = entity["StaticMeshComponent"];
			if (staticMeshComponent)
			{
				auto& src = deserializedEntity.AddComponent<StaticMeshComponent>();
				if (staticMeshComponent["StaticMesh"])
					src.StaticMesh = staticMeshComponent["StaticMesh"].as<uint64_t>();
				if (staticMeshComponent["Material"])
					src.Material = staticMeshComponent["Material"].as<uint64_t>();
				src.Color = staticMeshComponent["Color"].as<glm::vec4>(glm::vec4(0.8f, 0.3f, 0.2f, 1.0f));
				if (staticMeshComponent["Visible"])
					src.Visible = staticMeshComponent["Visible"].as<bool>();
			}

			auto directionalLightComponent = entity["DirectionalLightComponent"];
			if (directionalLightComponent)
			{
				auto& src = deserializedEntity.AddComponent<DirectionalLightComponent>();
				src.Radiance = directionalLightComponent["Radiance"].as<glm::vec3>(glm::vec3(1.0f));
				src.Intensity = directionalLightComponent["Intensity"].as<float>(1.0f);
			}

			auto rigidBody2DComponent = entity["RigidBody2DComponent"];
			if (rigidBody2DComponent)
			{
				auto& rb = deserializedEntity.AddComponent<RigidBody2DComponent>();
				rb.BodyType = (RigidBody2DComponent::Type)rigidBody2DComponent["BodyType"].as<int>(0);
				rb.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>(false);
				rb.GravityScale = rigidBody2DComponent["GravityScale"].as<float>(1.0f);
				rb.LinearDamping = rigidBody2DComponent["LinearDamping"].as<float>(0.01f);
				rb.AngularDamping = rigidBody2DComponent["AngularDamping"].as<float>(0.05f);
			}

			auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
			if (boxCollider2DComponent)
			{
				auto& box = deserializedEntity.AddComponent<BoxCollider2DComponent>();
				box.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>(glm::vec2(0.0f));
				box.Size = boxCollider2DComponent["Size"].as<glm::vec2>(glm::vec2(0.5f));
				box.Density = boxCollider2DComponent["Density"].as<float>(1.0f);
				box.Friction = boxCollider2DComponent["Friction"].as<float>(0.5f);
			}

			auto nativeScriptComponent = entity["NativeScriptComponent"];
			if (nativeScriptComponent)
			{
				auto& script = deserializedEntity.AddComponent<NativeScriptComponent>();
				if (nativeScriptComponent["ClassName"])
					script.ClassName = nativeScriptComponent["ClassName"].as<std::string>();
			}
		}
	}

	void SceneSerializer::SerializeRuntime([[maybe_unused]] const std::string& filepath)
	{
		HZ_CORE_ASSERT(false, "not implementation");
	}

	bool SceneSerializer::DeserializeFromYAML(const std::string& yamlString)
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["Scene"] && !data["Entities"])
			return false;

		if (data["Scene"])
		{
			auto entities = data["Entities"];
			if (entities)
				DeserializeEntities(entities, m_Scene);
			return true;
		}

		return false;
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		return DeserializeFromYAML(strStream.str());
	}

	bool SceneSerializer::DeserializeRuntime([[maybe_unused]] const std::string& filepath)
	{
		HZ_CORE_ASSERT(false, "not implementation");
		return false;
	}

	bool SceneSerializer::SerializeToAssetPack(FileStreamWriter& stream, AssetSerializationInfo& outInfo)
	{
		YAML::Emitter out;
		SerializeToYAML(out);

		outInfo.Offset = stream.GetStreamPosition();
		std::string yamlString = out.c_str();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	bool SceneSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::SceneInfo& sceneInfo)
	{
		stream.SetStreamPosition(sceneInfo.PackedOffset);
		std::string sceneYAML;
		stream.ReadString(sceneYAML);
		return DeserializeFromYAML(sceneYAML);
	}
}
