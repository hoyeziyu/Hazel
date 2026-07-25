#pragma once

#include "CSharpObject.h"
#include "ScriptEntityStorage.hpp"

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Core/UUID.h"

#include <Coral/Assembly.hpp>
#include <Coral/StableVector.hpp>
#include <Coral/Type.hpp>

namespace Coral {
	class HostInstance;
	class ManagedAssembly;
	class AssemblyLoadContext;
}

namespace Hazel {

	class Scene;
	class Project;

	struct FieldMetadata
	{
		std::string Name;
		DataType Type{};
		Coral::Type* ManagedType = nullptr;
		Buffer DefaultValue;

	private:
		template<typename T>
		void SetDefaultValue(Coral::ManagedObject& temp)
		{
			if (ManagedType->IsSZArray())
			{
				auto value = temp.GetFieldValue<Coral::Array<T>>(Name);
				DefaultValue = Buffer::Copy(value.Data(), value.ByteLength());
				Coral::Array<T>::Free(value);
			}
			else
			{
				DefaultValue.Allocate(sizeof(T));
				auto value = temp.GetFieldValue<T>(Name);
				DefaultValue.Write(&value, sizeof(T));
			}
		}

		friend class ScriptEngine;
	};

	struct ScriptMetadata
	{
		std::string FullName;
		std::unordered_map<uint32_t, FieldMetadata> Fields;
	};

	class ScriptEngine
	{
	public:
		static ScriptEngine& GetMutable();
		static const ScriptEngine& GetInstance() { return GetMutable(); }

		void InitializeHost();
		void ShutdownHost();

		void Initialize(const Ref<Project>& project);
		void Shutdown();
		void LoadProjectAssembly();

		void SetCurrentScene(const Ref<Scene>& scene) { m_CurrentScene = scene; }
		Ref<Scene> GetCurrentScene() const { return m_CurrentScene; }

		bool IsValidScript(UUID scriptID) const;
		UUID GetScriptIDByFullName(const std::string& fullName) const;
		const ScriptMetadata* GetScriptMetadata(UUID scriptID) const;
		const std::unordered_map<UUID, ScriptMetadata>& GetAllScripts() const { return m_ScriptMetadata; }

		CSharpObject Instantiate(UUID entityUUID, ScriptStorage& storage, uint64_t scriptEntityID);
		void DestroyInstance(UUID entityUUID, ScriptStorage& storage);

	private:
		struct AssemblyData
		{
			Coral::ManagedAssembly* Assembly = nullptr;
			std::unordered_map<UUID, Coral::Type*> CachedTypes;
		};

		void BuildAssemblyCache(AssemblyData* assemblyData);
		void ClearScriptMetadata();

	private:
		std::unique_ptr<Coral::HostInstance> m_Host;
		std::unique_ptr<Coral::AssemblyLoadContext> m_LoadContext;
		Scope<AssemblyData> m_CoreAssemblyData;
		Scope<AssemblyData> m_AppAssemblyData;
		std::unordered_map<UUID, ScriptMetadata> m_ScriptMetadata;
		Ref<Scene> m_CurrentScene;
		Coral::StableVector<Coral::ManagedObject> m_ManagedObjects;

		friend class Application;
		friend class Project;
		friend class Scene;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};

}
