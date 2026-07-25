#include "hzpch.h"
#include "ScriptGlue.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Script/ScriptEngine.h"

#include <Coral/Assembly.hpp>
#include <Coral/String.hpp>
#include <glm/glm.hpp>

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
	}

	void ScriptGlue::RegisterGlue(Coral::ManagedAssembly& coreAssembly)
	{
		HZ_ADD_INTERNAL_CALL(Scene_IsEntityValid);
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
