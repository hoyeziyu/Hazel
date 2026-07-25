#pragma once

#include "CSharpObject.h"

#include "Hazel/Core/Core.h"
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

	struct ScriptMetadata
	{
		std::string FullName;
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
		const ScriptMetadata* GetScriptMetadata(UUID scriptID) const;
		const std::unordered_map<UUID, ScriptMetadata>& GetAllScripts() const { return m_ScriptMetadata; }

		CSharpObject Instantiate(UUID scriptID, uint64_t entityUUID);
		void DestroyInstance(CSharpObject& instance);

	private:
		struct AssemblyData
		{
			Coral::ManagedAssembly* Assembly = nullptr;
			std::unordered_map<UUID, Coral::Type*> CachedTypes;
		};

		void BuildAssemblyCache(AssemblyData* assemblyData);

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
	};

}
