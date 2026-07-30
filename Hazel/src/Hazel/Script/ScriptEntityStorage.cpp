#include "hzpch.h"
#include "ScriptEntityStorage.hpp"
#include "ScriptEngine.h"

namespace Hazel {

	const char* DataTypeToString(DataType type)
	{
		switch (type)
		{
		case DataType::SByte: return "SByte";
		case DataType::Byte: return "Byte";
		case DataType::Short: return "Short";
		case DataType::UShort: return "UShort";
		case DataType::Int: return "Int";
		case DataType::UInt: return "UInt";
		case DataType::Long: return "Long";
		case DataType::ULong: return "ULong";
		case DataType::Float: return "Float";
		case DataType::Double: return "Double";
		case DataType::Vector2: return "Vector2";
		case DataType::Vector3: return "Vector3";
		case DataType::Bool: return "Bool";
		case DataType::Entity: return "Entity";
		case DataType::Prefab: return "Prefab";
		}
		return "Unknown";
	}

	DataType DataTypeFromString(const std::string& name)
	{
		if (name == "SByte") return DataType::SByte;
		if (name == "Byte") return DataType::Byte;
		if (name == "Short") return DataType::Short;
		if (name == "UShort") return DataType::UShort;
		if (name == "Int") return DataType::Int;
		if (name == "UInt") return DataType::UInt;
		if (name == "Long") return DataType::Long;
		if (name == "ULong") return DataType::ULong;
		if (name == "Float") return DataType::Float;
		if (name == "Double") return DataType::Double;
		if (name == "Vector2") return DataType::Vector2;
		if (name == "Vector3") return DataType::Vector3;
		if (name == "Bool") return DataType::Bool;
		if (name == "Entity") return DataType::Entity;
		if (name == "Prefab") return DataType::Prefab;
		return DataType::Int;
	}

	void ScriptStorage::InitializeEntityStorage(UUID scriptID, UUID entityID)
	{
		const auto& scriptEngine = ScriptEngine::GetInstance();
		HZ_CORE_ASSERT(scriptEngine.IsValidScript(scriptID), "Invalid script id");
		HZ_CORE_ASSERT(!EntityStorage.contains(entityID), "Script storage already exists");

		const auto* scriptMetadata = scriptEngine.GetScriptMetadata(scriptID);
		HZ_CORE_ASSERT(scriptMetadata, "Missing script metadata");

		auto& entityStorage = EntityStorage[entityID];
		entityStorage.ScriptID = scriptID;

		for (const auto& [fieldID, fieldMetadata] : scriptMetadata->Fields)
			InitializeFieldStorage(entityStorage, fieldID, fieldMetadata);
	}

	void ScriptStorage::ShutdownEntityStorage(UUID scriptID, UUID entityID)
	{
		const auto& scriptEngine = ScriptEngine::GetInstance();
		HZ_CORE_ASSERT(scriptEngine.IsValidScript(scriptID), "Invalid script id");
		HZ_CORE_ASSERT(EntityStorage.contains(entityID), "Missing script storage");

		for (auto& [fieldID, fieldStorage] : EntityStorage[entityID].Fields)
			fieldStorage.m_ValueBuffer.Release();

		EntityStorage.erase(entityID);
	}

	void ScriptStorage::SynchronizeStorage()
	{
		const auto& scriptEngine = ScriptEngine::GetInstance();

		for (auto& [entityID, entityStorage] : EntityStorage)
		{
			if (!scriptEngine.IsValidScript(entityStorage.ScriptID))
				continue;

			const auto* scriptMetadata = scriptEngine.GetScriptMetadata(entityStorage.ScriptID);
			if (!scriptMetadata)
				continue;

			for (const auto& [fieldID, fieldMetadata] : scriptMetadata->Fields)
			{
				if (entityStorage.Fields.contains(fieldID))
				{
					entityStorage.Fields[fieldID].m_Type = fieldMetadata.ManagedType;
					continue;
				}
				InitializeFieldStorage(entityStorage, fieldID, fieldMetadata);
			}
		}
	}

	void ScriptStorage::CopyTo(ScriptStorage& other) const
	{
		const auto& scriptEngine = ScriptEngine::GetInstance();

		for (const auto& [entityID, entityStorage] : EntityStorage)
		{
			if (!scriptEngine.IsValidScript(entityStorage.ScriptID))
			{
				HZ_CORE_WARN("[ScriptStorage] Cannot copy script data for script id {}", (uint64_t)entityStorage.ScriptID);
				continue;
			}

			const auto* scriptMetadata = scriptEngine.GetScriptMetadata(entityStorage.ScriptID);
			if (!scriptMetadata)
				continue;

			auto& otherStorage = other.EntityStorage[entityID];
			otherStorage.ScriptID = entityStorage.ScriptID;
			otherStorage.Instance = nullptr;

			for (const auto& [fieldID, fieldStorage] : entityStorage.Fields)
			{
				if (!scriptMetadata->Fields.contains(fieldID))
					continue;

				auto& otherFieldStorage = otherStorage.Fields[fieldID];
				otherFieldStorage.m_Name = fieldStorage.m_Name;
				otherFieldStorage.m_Type = fieldStorage.m_Type;
				otherFieldStorage.m_DataType = fieldStorage.m_DataType;
				otherFieldStorage.m_ValueBuffer = Buffer::Copy(fieldStorage.m_ValueBuffer);
				otherFieldStorage.m_Instance = nullptr;
			}
		}
	}

	void ScriptStorage::CopyEntityStorage(UUID entityID, UUID targetEntityID, ScriptStorage& targetStorage) const
	{
		if (!EntityStorage.contains(entityID) || !targetStorage.EntityStorage.contains(targetEntityID))
			return;

		const auto& scriptEngine = ScriptEngine::GetInstance();
		const auto& srcStorage = EntityStorage.at(entityID);

		if (!scriptEngine.IsValidScript(srcStorage.ScriptID))
			return;

		auto& dstStorage = targetStorage.EntityStorage.at(targetEntityID);
		if (dstStorage.ScriptID != srcStorage.ScriptID)
			return;

		const auto* scriptMetadata = scriptEngine.GetScriptMetadata(srcStorage.ScriptID);
		if (!scriptMetadata)
			return;

		dstStorage.Instance = nullptr;

		for (const auto& [fieldID, fieldStorage] : srcStorage.Fields)
		{
			if (!scriptMetadata->Fields.contains(fieldID))
				continue;

			auto& otherFieldStorage = dstStorage.Fields[fieldID];
			otherFieldStorage.m_Name = fieldStorage.m_Name;
			otherFieldStorage.m_Type = fieldStorage.m_Type;
			otherFieldStorage.m_DataType = fieldStorage.m_DataType;
			otherFieldStorage.m_ValueBuffer = Buffer::Copy(fieldStorage.m_ValueBuffer);
			otherFieldStorage.m_Instance = nullptr;
		}
	}

	void ScriptStorage::Clear()
	{
		for (auto& [entityID, entityStorage] : EntityStorage)
		{
			for (auto& [fieldID, fieldStorage] : entityStorage.Fields)
				fieldStorage.m_ValueBuffer.Release();
		}
		EntityStorage.clear();
	}

	void ScriptStorage::SetFieldStorage(UUID entityID, uint32_t fieldID, const std::string& name, DataType type, const Buffer& value)
	{
		auto& fieldStorage = EntityStorage[entityID].Fields[fieldID];
		fieldStorage.m_Name = name;
		fieldStorage.m_DataType = type;
		fieldStorage.m_ValueBuffer = Buffer::Copy(value);
		fieldStorage.m_Instance = nullptr;
	}

	void ScriptStorage::InitializeFieldStorage(EntityScriptStorage& storage, uint32_t fieldID, const FieldMetadata& fieldMetadata)
	{
		auto& fieldStorage = storage.Fields[fieldID];
		fieldStorage.m_Name = fieldMetadata.Name;
		fieldStorage.m_Type = fieldMetadata.ManagedType;
		fieldStorage.m_DataType = fieldMetadata.Type;

		if (fieldMetadata.DefaultValue.Data == nullptr || fieldMetadata.DefaultValue.Size == 0)
		{
			fieldStorage.m_ValueBuffer.Allocate(DataTypeSize(fieldMetadata.Type));
			fieldStorage.m_ValueBuffer.ZeroInitialize();
		}
		else
		{
			fieldStorage.m_ValueBuffer = Buffer::Copy(fieldMetadata.DefaultValue);
		}

		fieldStorage.m_Instance = nullptr;
	}

}
