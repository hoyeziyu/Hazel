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
#include "Hazel/Script/CSharpObject.h"

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

	struct HierarchyComponent
	{
		UUID Parent = 0;
		std::vector<UUID> Children;

		HierarchyComponent() = default;
		HierarchyComponent(const HierarchyComponent&) = default;
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

	struct RigidBody2DComponent
	{
		enum class Type { Static = 0, Dynamic, Kinematic };

		Type BodyType = Type::Static;
		bool FixedRotation = false;
		float GravityScale = 1.0f;
		float LinearDamping = 0.01f;
		float AngularDamping = 0.05f;

		// Runtime only (Box2D 3 body id bit pattern)
		uint64_t RuntimeBodyHandle = 0;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent& other) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset{ 0.0f, 0.0f };
		glm::vec2 Size{ 0.5f, 0.5f };
		float Density = 1.0f;
		float Friction = 0.5f;

		uint64_t RuntimeShapeHandle = 0;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent& other) = default;
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
		std::string ClassName;
		ScriptableEntity* Instance = nullptr;
		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct ScriptComponent
	{
		UUID ScriptID = 0;
		CSharpObject Instance;
	};

	struct AudioComponent
	{
		AssetHandle SoundConfig = 0;
		std::string FilePath;
		float Volume = 1.0f;
		bool PlayOnAwake = true;
		bool Loop = false;

		uint32_t RuntimeHandle = 0;

		AudioComponent() = default;
		AudioComponent(const AudioComponent& other) = default;
	};

	struct AnimationComponent
	{
		AssetHandle AnimationController = 0;
		std::vector<UUID> BoneEntities;
		bool EnableAnimation = true;
		float PlaybackSpeed = 1.0f;
		uint32_t StateIndex = 0;
		float AnimationTime = 0.0f;
		bool IsAnimationPlaying = false;
		bool EnableRootMotion = false;
		AssetHandle RootMotionTarget = 0;

		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent& other) = default;
	};

	struct SkinnedMeshComponent
	{
		AssetHandle StaticMesh = 0;
		AssetHandle Material = 0;
		glm::vec4 Color{ 0.8f, 0.3f, 0.2f, 1.0f };
		bool Visible = true;
		std::vector<UUID> BoneEntities;

		SkinnedMeshComponent() = default;
		SkinnedMeshComponent(const SkinnedMeshComponent& other) = default;
	};
}