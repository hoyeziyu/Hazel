#include "hzpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "Hazel/Core/Hash.h"
#include "Hazel/Project/Project.h"

#include <Coral/HostInstance.hpp>
#include <Coral/StringHelper.hpp>
#include <Coral/TypeCache.hpp>

namespace Hazel {

	static void OnCSharpException(std::string_view message)
	{
		HZ_CORE_ERROR("C# Exception: {}", message);
	}

	static void OnCoralMessage(std::string_view message, Coral::MessageLevel level)
	{
		switch (level)
		{
		case Coral::MessageLevel::Info:
			HZ_CORE_INFO("[Scripting] {}", std::string(message));
			break;
		case Coral::MessageLevel::Warning:
			HZ_CORE_WARN("[Scripting] {}", std::string(message));
			break;
		case Coral::MessageLevel::Error:
			HZ_CORE_ERROR("[Scripting] {}", std::string(message));
			break;
		}
	}

	ScriptEngine& ScriptEngine::GetMutable()
	{
		static ScriptEngine instance;
		return instance;
	}

	void ScriptEngine::InitializeHost()
	{
		if (m_Host)
			return;

		m_Host = std::make_unique<Coral::HostInstance>();
		const auto coralDir = (Project::GetRuntimeDirectory() / "DotNet").string();

		Coral::HostSettings settings{
			.CoralDirectory = coralDir,
			.MessageCallback = OnCoralMessage,
			.ExceptionCallback = OnCSharpException
		};

		const Coral::CoralInitStatus status = m_Host->Initialize(settings);
		if (status == Coral::CoralInitStatus::Success)
		{
			HZ_CORE_INFO("[Scripting] Coral host initialized (DotNet: {})", coralDir);
			return;
		}

		HZ_CORE_ERROR("[Scripting] Failed to initialize Coral host (status={}, DotNet path={}). Ensure .NET 9 runtime and Coral.Managed.dll are deployed.",
			(int)status, coralDir);
	}

	void ScriptEngine::ShutdownHost()
	{
		if (!m_Host)
			return;

		Shutdown();
		Coral::TypeCache::Get().Clear();
		m_Host->Shutdown();
		m_Host.reset();
	}

	void ScriptEngine::Initialize(const Ref<Project>& project)
	{
		(void)project;
		if (!m_Host)
			InitializeHost();

		m_LoadContext = std::make_unique<Coral::AssemblyLoadContext>(m_Host->CreateAssemblyLoadContext("HazelLoadContext"));

		const auto scriptCorePath = (Project::GetRuntimeDirectory() / "Resources" / "Scripts" / "Hazel-ScriptCore.dll").string();
		m_CoreAssemblyData = CreateScope<AssemblyData>();
		m_CoreAssemblyData->Assembly = &m_LoadContext->LoadAssembly(scriptCorePath);
		if (m_CoreAssemblyData->Assembly->GetLoadStatus() != Coral::AssemblyLoadStatus::Success)
		{
			HZ_CORE_ERROR("Failed to load Hazel-ScriptCore from {}", scriptCorePath);
			return;
		}

		BuildAssemblyCache(m_CoreAssemblyData.get());
		ScriptGlue::RegisterGlue(*m_CoreAssemblyData->Assembly);
		HZ_CORE_INFO("[Scripting] Loaded Hazel-ScriptCore");
	}

	void ScriptEngine::Shutdown()
	{
		m_ScriptMetadata.clear();
		m_ManagedObjects.Clear();
		m_AppAssemblyData.reset();
		m_CoreAssemblyData.reset();

		if (m_LoadContext && m_Host)
		{
			m_Host->UnloadAssemblyLoadContext(*m_LoadContext);
			m_LoadContext.reset();
		}

		Coral::TypeCache::Get().Clear();
	}

	void ScriptEngine::LoadProjectAssembly()
	{
		m_AppAssemblyData.reset();
		m_ScriptMetadata.clear();

		auto project = Project::GetActive();
		if (!project || !m_LoadContext)
			return;

		const auto filepath = std::filesystem::absolute(Project::GetScriptModuleFilePath());
		if (!std::filesystem::exists(filepath))
		{
			HZ_CORE_WARN("[Scripting] Game script assembly not found: {}", filepath.string());
			return;
		}

		m_AppAssemblyData = CreateScope<AssemblyData>();
		m_AppAssemblyData->Assembly = &m_LoadContext->LoadAssembly(filepath.string());
		if (m_AppAssemblyData->Assembly->GetLoadStatus() != Coral::AssemblyLoadStatus::Success)
		{
			HZ_CORE_ERROR("Failed to load game script assembly: {}", filepath.string());
			m_AppAssemblyData.reset();
			return;
		}

		BuildAssemblyCache(m_AppAssemblyData.get());
		HZ_CORE_INFO("[Scripting] Loaded game scripts from {}", filepath.string());
	}

	void ScriptEngine::BuildAssemblyCache(AssemblyData* assemblyData)
	{
		if (!assemblyData || !assemblyData->Assembly)
			return;

		auto& entityType = assemblyData->Assembly->GetType("Hazel.Entity");
		for (auto& type : assemblyData->Assembly->GetTypes())
		{
			std::string fullName = type->GetFullName();
			const Hazel::UUID scriptID = Hazel::UUID(static_cast<uint64_t>(Hash::GenerateFNVHash(fullName)));
			assemblyData->CachedTypes[scriptID] = type;

			if (type->IsSubclassOf(entityType))
			{
				auto& metadata = m_ScriptMetadata[scriptID];
				metadata.FullName = fullName;
			}
		}
	}

	bool ScriptEngine::IsValidScript(UUID scriptID) const
	{
		return m_AppAssemblyData
			&& m_AppAssemblyData->CachedTypes.contains(scriptID)
			&& m_ScriptMetadata.contains(scriptID);
	}

	const ScriptMetadata* ScriptEngine::GetScriptMetadata(UUID scriptID) const
	{
		auto it = m_ScriptMetadata.find(scriptID);
		return it != m_ScriptMetadata.end() ? &it->second : nullptr;
	}

	CSharpObject ScriptEngine::Instantiate(UUID scriptID, uint64_t entityUUID)
	{
		if (!IsValidScript(scriptID))
			return {};

		auto* type = m_AppAssemblyData->CachedTypes.at(scriptID);
		auto instance = type->CreateInstance(entityUUID);
		auto [index, handle] = m_ManagedObjects.Insert(std::move(instance));

		CSharpObject result;
		result.m_Handle = &handle;
		return result;
	}

	void ScriptEngine::DestroyInstance(CSharpObject& instance)
	{
		if (!instance.IsValid())
			return;

		instance.m_Handle->Destroy();
		instance.m_Handle = nullptr;
	}

}
