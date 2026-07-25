#include "hzpch.h"
#include "ScriptGlue.h"

#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Script/ScriptEngine.h"

#include <Coral/Assembly.hpp>
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
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HZ_ADD_INTERNAL_CALL(Log_LogMessage);
		coreAssembly.UploadInternalCalls();
	}

}
