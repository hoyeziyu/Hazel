#include "hzpch.h"
#include "ScriptGlue.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Physics2D/Physics2DScene.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Script/ScriptEngine.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Scene/Prefab.h"
#include "Hazel/Audio/AudioEngine.h"
#include "Hazel/Audio/AudioCommandRegistry.h"
#include "Hazel/Core/Hash.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Debug/RuntimeHUD.h"
#include "Hazel/Core/Time.h"

#include <Coral/Array.hpp>
#include <Coral/Assembly.hpp>
#include <Coral/String.hpp>
#include <Coral/Type.hpp>
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <cstring>
#include <functional>
#include <unordered_map>

namespace Hazel {

#define HZ_ADD_INTERNAL_CALL(icall) coreAssembly.AddInternalCall("Hazel.InternalCalls", #icall, (void*)InternalCalls::icall)

	static std::unordered_map<Coral::TypeId, std::function<bool(Entity&)>> s_HasComponentFuncs;

	template<typename TComponent>
	static void RegisterManagedComponent(Coral::ManagedAssembly& coreAssembly, const char* managedName)
	{
		auto& type = coreAssembly.GetType(managedName);
		if (!type)
		{
			HZ_CORE_WARN("[Scripting] Missing C# component type: {}", managedName);
			return;
		}

		s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return entity.HasComponent<TComponent>(); };
	}

	enum class LogLevel : int32_t
	{
		Trace = 1 << 0,
		Debug = 1 << 1,
		Info = 1 << 2,
		Warn = 1 << 3,
		Error = 1 << 4,
		Critical = 1 << 5
	};

	namespace InternalCalls {

		uint64_t Scene_InstantiatePrefabWithTransform(uint64_t prefabHandle, glm::vec3* translation, glm::vec3* rotation, glm::vec3* scale);
		uint64_t Scene_InstantiatePrefabWithTranslation(uint64_t prefabHandle, glm::vec3* translation);
		uint64_t Scene_InstantiateChildPrefabWithTransform(uint64_t parentID, uint64_t prefabHandle, glm::vec3* translation, glm::vec3* rotation, glm::vec3* scale);
		uint64_t Scene_InstantiateChildPrefabWithTranslation(uint64_t parentID, uint64_t prefabHandle, glm::vec3* translation);

		static b2BodyId DecodeBody(uint64_t handle)
		{
			b2BodyId body = b2_nullBodyId;
			if (handle != 0)
				std::memcpy(&body, &handle, sizeof(b2BodyId));
			return body;
		}

		static Entity GetEntity(uint64_t entityID)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			HZ_CORE_ASSERT(scene, "No active scene for script call");
			return scene->GetEntityWithUUID(entityID);
		}

		bool Scene_IsEntityValid(uint64_t entityID)
		{
			if (entityID == 0)
				return false;
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
				return false;
			return (bool)scene->GetEntityWithUUID(entityID);
		}

		Coral::String TagComponent_GetTag(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<TagComponent>())
				return Coral::String::New("");
			return Coral::String::New(entity.GetComponent<TagComponent>().Tag);
		}

		void TagComponent_SetTag(uint64_t entityID, Coral::String tag)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<TagComponent>())
			{
				Coral::String::Free(tag);
				return;
			}
			entity.GetComponent<TagComponent>().Tag = tag;
			Coral::String::Free(tag);
		}

		void TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !outTranslation)
				return;
			*outTranslation = entity.GetComponent<TransformComponent>().Translation;
		}

		void TransformComponent_SetTranslation(uint64_t entityID, glm::vec3* inTranslation)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !inTranslation)
				return;
			entity.GetComponent<TransformComponent>().Translation = *inTranslation;
		}

		void TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !outRotation)
				return;
			*outRotation = entity.GetComponent<TransformComponent>().Rotation;
		}

		void TransformComponent_SetRotation(uint64_t entityID, glm::vec3* inRotation)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !inRotation)
				return;
			entity.GetComponent<TransformComponent>().Rotation = *inRotation;
		}

		void TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !outScale)
				return;
			*outScale = entity.GetComponent<TransformComponent>().Scale;
		}

		void TransformComponent_SetScale(uint64_t entityID, glm::vec3* inScale)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !inScale)
				return;
			entity.GetComponent<TransformComponent>().Scale = *inScale;
		}

		bool Input_IsKeyPressed(int32_t keycode)
		{
			return Input::IsKeyPressed(keycode);
		}

		bool Input_IsMouseButtonPressed(int32_t button)
		{
			return Input::IsMouseButtonPressed(button);
		}

		void Input_GetMousePosition(glm::vec2* outPosition)
		{
			if (!outPosition)
				return;
			auto [x, y] = Input::GetMousePosition();
			*outPosition = { x, y };
		}

		void Log_LogMessage(LogLevel level, Coral::String message)
		{
			std::string text = message;
			switch (level)
			{
			case LogLevel::Trace: HZ_TRACE(text); break;
			case LogLevel::Debug: HZ_INFO(text); break;
			case LogLevel::Info: HZ_INFO(text); break;
			case LogLevel::Warn: HZ_WARN(text); break;
			case LogLevel::Error: HZ_ERROR(text); break;
			case LogLevel::Critical: HZ_CORE_CRITICAL(text); break;
			}
			Coral::String::Free(message);
		}

		uint64_t Scene_FindEntityByTag(Coral::String tag)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
			{
				Coral::String::Free(tag);
				return 0;
			}

			Entity entity = scene->TryGetEntityWithTag(tag);
			Coral::String::Free(tag);
			return entity ? entity.GetUUID() : UUID(0);
		}

		uint64_t Scene_CreateEntity(Coral::String tag)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
			{
				Coral::String::Free(tag);
				return 0;
			}

			Entity entity = scene->CreateEntity(tag);
			Coral::String::Free(tag);
			return entity.GetUUID();
		}

		void Scene_DestroyEntity(uint64_t entityID)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
				return;

			Entity entity = scene->GetEntityWithUUID(entityID);
			if (entity)
				scene->DestroyEntity(entity);
		}

		Coral::Array<uint64_t> Scene_GetEntities()
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
				return Coral::Array<uint64_t>::New(0);

			const auto ids = scene->GetAllEntityUUIDs();
			auto result = Coral::Array<uint64_t>::New((int32_t)ids.size());
			for (int32_t i = 0; i < (int32_t)ids.size(); i++)
				result[i] = ids[i];
			return result;
		}

		int32_t RigidBody2DComponent_GetBodyType(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
				return 0;
			return (int32_t)entity.GetComponent<RigidBody2DComponent>().BodyType;
		}

		void RigidBody2DComponent_SetBodyType(uint64_t entityID, int32_t bodyType)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
				return;
			entity.GetComponent<RigidBody2DComponent>().BodyType = (RigidBody2DComponent::Type)bodyType;
		}

		void RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity)
		{
			if (!outVelocity)
				return;

			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
			{
				*outVelocity = {};
				return;
			}

			b2BodyId bodyId = DecodeBody(entity.GetComponent<RigidBody2DComponent>().RuntimeBodyHandle);
			if (B2_IS_NULL(bodyId))
			{
				*outVelocity = {};
				return;
			}

			const b2Vec2 velocity = b2Body_GetLinearVelocity(bodyId);
			*outVelocity = { velocity.x, velocity.y };
		}

		void RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, glm::vec2* inVelocity)
		{
			if (!inVelocity)
				return;

			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
				return;

			b2BodyId bodyId = DecodeBody(entity.GetComponent<RigidBody2DComponent>().RuntimeBodyHandle);
			if (B2_IS_NULL(bodyId))
				return;

			b2Body_SetLinearVelocity(bodyId, { inVelocity->x, inVelocity->y });
		}

		float RigidBody2DComponent_GetGravityScale(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
				return 0.0f;
			return entity.GetComponent<RigidBody2DComponent>().GravityScale;
		}

		void RigidBody2DComponent_SetGravityScale(uint64_t entityID, float gravityScale)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
				return;

			auto& rb = entity.GetComponent<RigidBody2DComponent>();
			rb.GravityScale = gravityScale;

			b2BodyId bodyId = DecodeBody(rb.RuntimeBodyHandle);
			if (!B2_IS_NULL(bodyId))
				b2Body_SetGravityScale(bodyId, gravityScale);
		}

		void RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* offset, bool wake)
		{
			(void)offset;
			(void)wake;
			if (!impulse)
				return;

			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<RigidBody2DComponent>())
				return;

			Physics2DScene::ApplyLinearImpulse(entity.GetComponent<RigidBody2DComponent>(), *impulse);
		}

		uint32_t AnimationComponent_GetStateIndex(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AnimationComponent>())
				return 0;
			return entity.GetComponent<AnimationComponent>().StateIndex;
		}

		void AnimationComponent_SetStateIndex(uint64_t entityID, uint32_t stateIndex)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AnimationComponent>())
				return;
			entity.GetComponent<AnimationComponent>().StateIndex = stateIndex;
		}

		float AnimationComponent_GetAnimationTime(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AnimationComponent>())
				return 0.0f;
			return entity.GetComponent<AnimationComponent>().AnimationTime;
		}

		void AnimationComponent_SetAnimationTime(uint64_t entityID, float animationTime)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AnimationComponent>())
				return;
			entity.GetComponent<AnimationComponent>().AnimationTime = animationTime;
		}

		bool AnimationComponent_GetIsAnimationPlaying(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AnimationComponent>())
				return false;
			return entity.GetComponent<AnimationComponent>().IsAnimationPlaying;
		}

		void AnimationComponent_SetIsAnimationPlaying(uint64_t entityID, bool isPlaying)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AnimationComponent>())
				return;
			entity.GetComponent<AnimationComponent>().IsAnimationPlaying = isPlaying;
		}

		uint64_t Entity_GetParent(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<HierarchyComponent>())
				return 0;
			return entity.GetComponent<HierarchyComponent>().Parent;
		}

		void Entity_SetParent(uint64_t childID, uint64_t parentID)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
				return;

			Entity child = scene->GetEntityWithUUID(childID);
			if (!child)
				return;

			Entity parent = parentID != 0 ? scene->GetEntityWithUUID(parentID) : Entity{};
			scene->SetParent(child, parent);
		}

		Coral::Array<uint64_t> Entity_GetChildren(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<HierarchyComponent>())
				return Coral::Array<uint64_t>::New(0);

			const auto& children = entity.GetComponent<HierarchyComponent>().Children;
			auto result = Coral::Array<uint64_t>::New((int32_t)children.size());
			for (int32_t i = 0; i < (int32_t)children.size(); i++)
				result[i] = children[i];
			return result;
		}

		bool Entity_HasComponent(uint64_t entityID, Coral::ReflectionType componentType)
		{
			Entity entity = GetEntity(entityID);
			if (!entity)
				return false;

			Coral::Type& type = componentType;
			if (!type)
				return false;

			auto it = s_HasComponentFuncs.find(type.GetTypeId());
			if (it == s_HasComponentFuncs.end())
				return false;

			auto func = it->second;
			return func(entity);
		}

		uint64_t Scene_InstantiatePrefab(uint64_t prefabHandle)
		{
			return Scene_InstantiatePrefabWithTranslation(prefabHandle, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTranslation(uint64_t prefabHandle, glm::vec3* translation)
		{
			return Scene_InstantiatePrefabWithTransform(prefabHandle, translation, nullptr, nullptr);
		}

		uint64_t Scene_InstantiatePrefabWithTransform(uint64_t prefabHandle, glm::vec3* translation, glm::vec3* rotation, glm::vec3* scale)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
				return 0;

			auto prefab = AssetManager::GetAsset<Prefab>(AssetHandle(prefabHandle));
			if (!prefab)
			{
				HZ_CORE_WARN("[Scripting] Cannot instantiate prefab handle {}", prefabHandle);
				return 0;
			}

			Entity entity = scene->Instantiate(prefab, translation, rotation, scale);
			return entity ? entity.GetUUID() : UUID(0);
		}

		uint64_t Scene_InstantiateChildPrefabWithTranslation(uint64_t parentID, uint64_t prefabHandle, glm::vec3* translation)
		{
			return Scene_InstantiateChildPrefabWithTransform(parentID, prefabHandle, translation, nullptr, nullptr);
		}

		uint64_t Scene_InstantiateChildPrefabWithTransform(uint64_t parentID, uint64_t prefabHandle, glm::vec3* translation, glm::vec3* rotation, glm::vec3* scale)
		{
			Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
			if (!scene)
				return 0;

			Entity parent = scene->GetEntityWithUUID(parentID);
			if (!parent)
				return 0;

			auto prefab = AssetManager::GetAsset<Prefab>(AssetHandle(prefabHandle));
			if (!prefab)
				return 0;

			Entity entity = scene->InstantiateChild(prefab, parent, translation, rotation, scale);
			return entity ? entity.GetUUID() : UUID(0);
		}

		Coral::ManagedObject ScriptComponent_GetInstance(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<ScriptComponent>())
				return {};

			const auto& component = entity.GetComponent<ScriptComponent>();
			if (!component.Instance.IsValid())
				return {};

			return *component.Instance.GetManagedObject();
		}

		uint32_t AudioCommandID_Constructor(Coral::String commandName)
		{
			const uint32_t id = Hash::GenerateFNVHash(std::string(commandName));
			Coral::String::Free(commandName);
			return id;
		}

		uint32_t Audio_PostEventFromAC(uint32_t commandId, uint64_t entityID)
		{
			AssetHandle soundConfig = 0;
			if (!AudioCommandRegistry::Get().TryGetSoundConfig(commandId, soundConfig))
			{
				HZ_CORE_WARN("[Audio] Unknown audio command id {}", commandId);
				return 0;
			}

			Entity entity = GetEntity(entityID);
			if (entity && entity.HasComponent<AudioComponent>())
			{
				auto& audio = entity.GetComponent<AudioComponent>();
				audio.SoundConfig = soundConfig;
			}

			AudioEngine::Get().PlaySoundConfig(soundConfig);
			return commandId;
		}

		bool AudioComponent_GetPlayOnAwake(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AudioComponent>())
				return false;
			return entity.GetComponent<AudioComponent>().PlayOnAwake;
		}

		void AudioComponent_SetPlayOnAwake(uint64_t entityID, bool value)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AudioComponent>())
				return;
			entity.GetComponent<AudioComponent>().PlayOnAwake = value;
		}

		float AudioComponent_GetVolume(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AudioComponent>())
				return 1.0f;
			return entity.GetComponent<AudioComponent>().Volume;
		}

		void AudioComponent_SetVolume(uint64_t entityID, float value)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<AudioComponent>())
				return;
			entity.GetComponent<AudioComponent>().Volume = value;
		}

		float CameraComponent_GetVerticalFOV(uint64_t entityID)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<CameraComponent>())
				return 45.0f;
			return glm::degrees(entity.GetComponent<CameraComponent>().Camera.GetPerspectiveVerticalFOV());
		}

		void CameraComponent_SetVerticalFOV(uint64_t entityID, float verticalFovDegrees)
		{
			Entity entity = GetEntity(entityID);
			if (!entity || !entity.HasComponent<CameraComponent>())
				return;
			entity.GetComponent<CameraComponent>().Camera.SetPerspectiveVerticalFOV(glm::radians(verticalFovDegrees));
		}

		void RuntimeHUD_SetLine(int32_t index, Coral::String text)
		{
			if (index < 0)
			{
				Coral::String::Free(text);
				return;
			}
			RuntimeHUD::SetLine((size_t)index, std::string(text));
			Coral::String::Free(text);
		}

		void RuntimeHUD_Clear()
		{
			RuntimeHUD::Clear();
		}

		void RuntimeWorldLabel_Set(int32_t index, glm::vec3* position, Coral::String text)
		{
			if (index < 0 || !position)
			{
				Coral::String::Free(text);
				return;
			}

			RuntimeHUD::SetWorldLabel((size_t)index, *position, std::string(text));
			Coral::String::Free(text);
		}

		void RuntimeWorldLabel_Clear()
		{
			RuntimeHUD::ClearWorldLabels();
		}

		float Time_GetTimeScale()
		{
			return Time::GetTimeScale();
		}

		void Time_SetTimeScale(float timeScale)
		{
			Time::SetTimeScale(timeScale);
		}
	}

	void ScriptGlue::RegisterGlue(Coral::ManagedAssembly& coreAssembly)
	{
		s_HasComponentFuncs.clear();

		RegisterManagedComponent<TagComponent>(coreAssembly, "Hazel.TagComponent");
		RegisterManagedComponent<TransformComponent>(coreAssembly, "Hazel.TransformComponent");
		RegisterManagedComponent<RigidBody2DComponent>(coreAssembly, "Hazel.RigidBody2DComponent");
		RegisterManagedComponent<AnimationComponent>(coreAssembly, "Hazel.AnimationComponent");
		RegisterManagedComponent<AudioComponent>(coreAssembly, "Hazel.AudioComponent");
		RegisterManagedComponent<ScriptComponent>(coreAssembly, "Hazel.ScriptComponent");
		RegisterManagedComponent<CameraComponent>(coreAssembly, "Hazel.CameraComponent");
		RegisterManagedComponent<SkinnedMeshComponent>(coreAssembly, "Hazel.SkinnedMeshComponent");

		HZ_ADD_INTERNAL_CALL(Scene_IsEntityValid);
		HZ_ADD_INTERNAL_CALL(Scene_FindEntityByTag);
		HZ_ADD_INTERNAL_CALL(Scene_CreateEntity);
		HZ_ADD_INTERNAL_CALL(Scene_DestroyEntity);
		HZ_ADD_INTERNAL_CALL(Scene_GetEntities);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_GetLinearVelocity);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_SetLinearVelocity);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_GetGravityScale);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_SetGravityScale);
		HZ_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(TagComponent_GetTag);
		HZ_ADD_INTERNAL_CALL(TagComponent_SetTag);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetScale);
		HZ_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		HZ_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		HZ_ADD_INTERNAL_CALL(Input_GetMousePosition);
		HZ_ADD_INTERNAL_CALL(Log_LogMessage);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_GetStateIndex);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_SetStateIndex);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_GetAnimationTime);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_SetAnimationTime);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_GetIsAnimationPlaying);
		HZ_ADD_INTERNAL_CALL(AnimationComponent_SetIsAnimationPlaying);
		HZ_ADD_INTERNAL_CALL(Entity_GetParent);
		HZ_ADD_INTERNAL_CALL(Entity_SetParent);
		HZ_ADD_INTERNAL_CALL(Entity_GetChildren);
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Scene_InstantiatePrefab);
		HZ_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTranslation);
		HZ_ADD_INTERNAL_CALL(Scene_InstantiatePrefabWithTransform);
		HZ_ADD_INTERNAL_CALL(Scene_InstantiateChildPrefabWithTranslation);
		HZ_ADD_INTERNAL_CALL(Scene_InstantiateChildPrefabWithTransform);
		HZ_ADD_INTERNAL_CALL(ScriptComponent_GetInstance);
		HZ_ADD_INTERNAL_CALL(AudioCommandID_Constructor);
		HZ_ADD_INTERNAL_CALL(Audio_PostEventFromAC);
		HZ_ADD_INTERNAL_CALL(AudioComponent_GetPlayOnAwake);
		HZ_ADD_INTERNAL_CALL(AudioComponent_SetPlayOnAwake);
		HZ_ADD_INTERNAL_CALL(AudioComponent_GetVolume);
		HZ_ADD_INTERNAL_CALL(AudioComponent_SetVolume);
		HZ_ADD_INTERNAL_CALL(CameraComponent_GetVerticalFOV);
		HZ_ADD_INTERNAL_CALL(CameraComponent_SetVerticalFOV);
		HZ_ADD_INTERNAL_CALL(RuntimeHUD_SetLine);
		HZ_ADD_INTERNAL_CALL(RuntimeHUD_Clear);
		HZ_ADD_INTERNAL_CALL(RuntimeWorldLabel_Set);
		HZ_ADD_INTERNAL_CALL(RuntimeWorldLabel_Clear);
		HZ_ADD_INTERNAL_CALL(Time_GetTimeScale);
		HZ_ADD_INTERNAL_CALL(Time_SetTimeScale);
		coreAssembly.UploadInternalCalls();
	}

}
