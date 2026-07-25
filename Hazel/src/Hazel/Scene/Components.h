#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Hazel/Core/UUID.h"
#include "Hazel/Asset/Asset.h"
#include "SceneCamera.h"
#include "ScriptableEntity.h"

namespace Hazel {

	struct IDComponent
	{
		UUID ID = 0;
	};

	struct PrefabComponent
	{
		UUID PrefabID = 0;
		UUID EntityID = 0;

		PrefabComponent() = default;
		PrefabComponent(UUID prefabID, UUID entityID)
			: PrefabID(prefabID), EntityID(entityID) {}
	};

	struct TagComponent
	{
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};


	struct TransformComponent
	{
		glm::mat4 Transform = { 1.0f };
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		TransformComponent() = default;
		TransformComponent(const TransformComponent& transform) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		//operator glm::mat4& () { return Transform; }
		//operator const glm::mat4& () const { return Transform; }		// 隐式的强制转换运算符 重载
		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};


	struct SpriteRendererComponent
	{
		AssetHandle Texture = 0;
		glm::vec4 Color{ 1.0f,1.0f,1.0f,1.0f };
		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4 & color)
			: Color(color) {}
	};

	struct MeshRendererComponent
	{
		glm::vec4 Color{ 0.8f, 0.3f, 0.2f, 1.0f };
		bool Visible = true;

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct StaticMeshComponent
	{
		AssetHandle StaticMesh = 0;
		AssetHandle Material = 0;
		glm::vec4 Color{ 0.8f, 0.3f, 0.2f, 1.0f };
		bool Visible = true;

		StaticMeshComponent() = default;
		StaticMeshComponent(const StaticMeshComponent&) = default;
		StaticMeshComponent(AssetHandle staticMesh)
			: StaticMesh(staticMesh) {}
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance{ 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(const glm::vec3& radiance, float intensity = 1.0f)
			: Radiance(radiance), Intensity(intensity) {}
	};


	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;	// 是否是主相机，todo: 考虑移动到场景
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		
	};


	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;
		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};
}