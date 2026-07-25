#include "hzpch.h"
#include "ScriptGlue.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Physics2D/Physics2DScene.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Script/ScriptEngine.h"

#include <Coral/Array.hpp>
#include <Coral/Assembly.hpp>
#include <Coral/String.hpp>
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <cstring>

namespace Hazel {

#define HZ_ADD_INTERNAL_CALL(icall) coreAssembly.AddInternalCall("Hazel.InternalCalls", #icall, (void*)InternalCalls::icall)

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
	}

	void ScriptGlue::RegisterGlue(Coral::ManagedAssembly& coreAssembly)
	{
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
		coreAssembly.UploadInternalCalls();
	}

}
