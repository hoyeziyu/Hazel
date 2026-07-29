#include "hzpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "Hazel/Core/Hash.h"
#include "Hazel/Project/Project.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <Coral/Attribute.hpp>
#include <Coral/HostInstance.hpp>
#include <Coral/StringHelper.hpp>
#include <Coral/TypeCache.hpp>

namespace Hazel {

	namespace {

		std::unordered_map<std::string, DataType> s_DataTypeLookup = {
			{ "System.SByte", DataType::SByte },
			{ "System.Byte", DataType::Byte },
			{ "System.Int16", DataType::Short },
			{ "System.UInt16", DataType::UShort },
			{ "System.Int32", DataType::Int },
			{ "System.UInt32", DataType::UInt },
			{ "System.Int64", DataType::Long },
			{ "System.UInt64", DataType::ULong },
			{ "System.Single", DataType::Float },
			{ "System.Double", DataType::Double },
			{ "Hazel.Vector2", DataType::Vector2 },
			{ "Hazel.Vector3", DataType::Vector3 },
			{ "System.Boolean", DataType::Bool },
			{ "Hazel.Entity", DataType::Entity },
		};

	}

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

	void ScriptEngine::ClearScriptMetadata()
	{
		for (auto& [scriptID, scriptMetadata] : m_ScriptMetadata)
		{
			for (auto& [fieldID, fieldMetadata] : scriptMetadata.Fields)
				fieldMetadata.DefaultValue.Release();
		}
		m_ScriptMetadata.clear();
	}

	void ScriptEngine::InitializeHost()
	{
		if (m_Host)
			return;

		m_HostReady = false;
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
			m_HostReady = true;
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
		m_HostReady = false;
	}

	void ScriptEngine::Initialize(const Ref<Project>& project)
	{
		(void)project;
		if (!m_Host)
			InitializeHost();

		if (!m_HostReady)
			return;

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
		ClearScriptMetadata();
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
		ClearScriptMetadata();

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

			if (!type->IsSubclassOf(entityType))
				continue;

			auto& metadata = m_ScriptMetadata[scriptID];
			metadata.FullName = fullName;

			auto temp = type->CreateInstance();
			for (auto& fieldInfo : type->GetFields())
			{
				Coral::ScopedString fieldName = fieldInfo.GetName();
				std::string fieldNameStr = fieldName;

				if (fieldNameStr.find("k__BackingField") != std::string::npos)
					continue;
				if (fieldNameStr == "ID")
					continue;

				auto* fieldType = &fieldInfo.GetType();
				if (fieldType->IsSZArray())
					fieldType = &fieldType->GetElementType();

				Coral::ScopedString typeName = fieldType->GetFullName();
				if (!s_DataTypeLookup.contains(typeName))
					continue;

				if (fieldInfo.GetAccessibility() != Coral::TypeAccessibility::Public)
				{
					auto attributes = fieldInfo.GetAttributes();
					auto found = std::find_if(attributes.begin(), attributes.end(), [](Coral::Attribute& attribute)
					{
						Coral::ScopedString name = attribute.GetType().GetFullName();
						return name == "Hazel.ShowInEditorAttribute";
					});
					if (found == attributes.end())
						continue;
				}

				const auto fullFieldName = std::format("{}.{}", fullName, fieldNameStr);
				const uint32_t fieldID = Hash::GenerateFNVHash(fullFieldName);

				auto& fieldMetadata = metadata.Fields[fieldID];
				fieldMetadata.Name = fieldName;
				fieldMetadata.Type = s_DataTypeLookup.at(typeName);
				fieldMetadata.ManagedType = &fieldInfo.GetType();

				switch (fieldMetadata.Type)
				{
				case DataType::SByte: fieldMetadata.SetDefaultValue<int8_t>(temp); break;
				case DataType::Byte: fieldMetadata.SetDefaultValue<uint8_t>(temp); break;
				case DataType::Short: fieldMetadata.SetDefaultValue<int16_t>(temp); break;
				case DataType::UShort: fieldMetadata.SetDefaultValue<uint16_t>(temp); break;
				case DataType::Int: fieldMetadata.SetDefaultValue<int32_t>(temp); break;
				case DataType::UInt: fieldMetadata.SetDefaultValue<uint32_t>(temp); break;
				case DataType::Long: fieldMetadata.SetDefaultValue<int64_t>(temp); break;
				case DataType::ULong: fieldMetadata.SetDefaultValue<uint64_t>(temp); break;
				case DataType::Float: fieldMetadata.SetDefaultValue<float>(temp); break;
				case DataType::Double: fieldMetadata.SetDefaultValue<double>(temp); break;
				case DataType::Vector2: fieldMetadata.SetDefaultValue<glm::vec2>(temp); break;
				case DataType::Vector3: fieldMetadata.SetDefaultValue<glm::vec3>(temp); break;
				case DataType::Bool: fieldMetadata.SetDefaultValue<Coral::Bool32>(temp); break;
				case DataType::Entity: break;
				}
			}

			temp.Destroy();
		}
	}

	bool ScriptEngine::IsValidScript(UUID scriptID) const
	{
		return m_AppAssemblyData
			&& m_AppAssemblyData->CachedTypes.contains(scriptID)
			&& m_ScriptMetadata.contains(scriptID);
	}

	UUID ScriptEngine::GetScriptIDByFullName(const std::string& fullName) const
	{
		const Hazel::UUID hashedID = Hazel::UUID(static_cast<uint64_t>(Hash::GenerateFNVHash(fullName)));
		if (IsValidScript(hashedID))
			return hashedID;

		for (const auto& [scriptID, metadata] : m_ScriptMetadata)
		{
			if (metadata.FullName == fullName && IsValidScript(scriptID))
				return scriptID;
		}

		return 0;
	}

	const ScriptMetadata* ScriptEngine::GetScriptMetadata(UUID scriptID) const
	{
		auto it = m_ScriptMetadata.find(scriptID);
		return it != m_ScriptMetadata.end() ? &it->second : nullptr;
	}

	CSharpObject ScriptEngine::Instantiate(UUID entityUUID, ScriptStorage& storage, uint64_t scriptEntityID)
	{
		HZ_CORE_ASSERT(storage.EntityStorage.contains(entityUUID), "Missing entity script storage");

		auto& entityStorage = storage.EntityStorage.at(entityUUID);
		if (!IsValidScript(entityStorage.ScriptID))
			return {};

		auto* type = m_AppAssemblyData->CachedTypes.at(entityStorage.ScriptID);
		auto instance = type->CreateInstance(scriptEntityID);
		auto [index, handle] = m_ManagedObjects.Insert(std::move(instance));
		entityStorage.Instance = &handle;

		auto& editorAssignableAttribType = m_CoreAssemblyData->Assembly->GetType("Hazel.EditorAssignableAttribute");
		const auto* scriptMetadata = GetScriptMetadata(entityStorage.ScriptID);
		HZ_CORE_ASSERT(scriptMetadata, "Missing script metadata");

		for (auto& [fieldID, fieldStorage] : entityStorage.Fields)
		{
			const auto& fieldMetadata = scriptMetadata->Fields.at(fieldID);

			if (fieldMetadata.ManagedType->HasAttribute(editorAssignableAttribType))
			{
				Coral::ManagedObject value = fieldMetadata.ManagedType->CreateInstance(fieldStorage.GetValue<uint64_t>());
				handle.SetFieldValue(fieldStorage.GetName(), value);
				value.Destroy();
			}
			else if (fieldMetadata.ManagedType->IsSZArray())
			{
				struct ArrayContainer
				{
					void* Data;
					int32_t Length;
				} array;

				array.Data = fieldStorage.m_ValueBuffer.Data;
				array.Length = static_cast<int32_t>(fieldStorage.GetLength());
				handle.SetFieldValueRaw(fieldStorage.GetName(), &array);
			}
			else
			{
				handle.SetFieldValueRaw(fieldStorage.GetName(), fieldStorage.m_ValueBuffer.Data);
			}

			fieldStorage.m_Instance = &handle;
		}

		CSharpObject result;
		result.m_Handle = &handle;
		return result;
	}

	void ScriptEngine::DestroyInstance(UUID entityUUID, ScriptStorage& storage)
	{
		if (!storage.EntityStorage.contains(entityUUID))
			return;

		auto& entityStorage = storage.EntityStorage.at(entityUUID);
		if (!entityStorage.Instance)
			return;

		for (auto& [fieldID, fieldStorage] : entityStorage.Fields)
			fieldStorage.m_Instance = nullptr;

		entityStorage.Instance->Destroy();
		entityStorage.Instance = nullptr;
	}

}
