#pragma once

#include "Hazel/Core/Buffer.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Core/UUID.h"

#include <Coral/ManagedObject.hpp>
#include <Coral/Type.hpp>
#include <Coral/Array.hpp>

#include <string>
#include <unordered_map>

namespace Hazel {

	enum class DataType
	{
		SByte,
		Byte,
		Short,
		UShort,
		Int,
		UInt,
		Long,
		ULong,
		Float,
		Double,
		Vector2,
		Vector3,
		Bool,
		Entity,
		Prefab
	};

	inline uint64_t DataTypeSize(DataType type)
	{
		switch (type)
		{
		case DataType::SByte: return sizeof(int8_t);
		case DataType::Byte: return sizeof(uint8_t);
		case DataType::Short: return sizeof(int16_t);
		case DataType::UShort: return sizeof(uint16_t);
		case DataType::Int: return sizeof(int32_t);
		case DataType::UInt: return sizeof(uint32_t);
		case DataType::Long: return sizeof(int64_t);
		case DataType::ULong: return sizeof(uint64_t);
		case DataType::Float: return sizeof(float);
		case DataType::Double: return sizeof(double);
		case DataType::Vector2: return sizeof(float) * 2;
		case DataType::Vector3: return sizeof(float) * 3;
		case DataType::Bool: return sizeof(Coral::Bool32);
		case DataType::Entity: return sizeof(UUID);
		case DataType::Prefab: return sizeof(UUID);
		}
		return 0;
	}

	const char* DataTypeToString(DataType type);
	DataType DataTypeFromString(const std::string& name);

	struct FieldMetadata;

	class FieldStorage
	{
	public:
		std::string_view GetName() const { return m_Name; }
		DataType GetType() const { return m_DataType; }
		bool IsArray() const { return m_Type && m_Type->IsSZArray(); }

		uint64_t GetLength() const
		{
			if (m_Instance)
				return m_ValueBuffer.Size / DataTypeSize(m_DataType);
			return m_ValueBuffer.Size / DataTypeSize(m_DataType);
		}

		template<typename T>
		T GetValue() const
		{
			return m_Instance ? m_Instance->GetFieldValue<T>(m_Name) : m_ValueBuffer.Read<T>();
		}

		template<typename T>
		T GetValue(uint32_t index) const
		{
			if (m_Instance)
			{
				using ValueArray = Coral::Array<T>;
				auto arr = m_Instance->GetFieldValue<ValueArray>(m_Name);
				T value = arr[index];
				ValueArray::Free(arr);
				return value;
			}
			return m_ValueBuffer.Read<T>(index * sizeof(T));
		}

		template<typename T>
		void SetValue(const T& value)
		{
			if (m_Instance)
				m_Instance->SetFieldValue(m_Name, value);
			else
				m_ValueBuffer.Write(&value, sizeof(T));
		}

		template<typename T>
		void SetValue(const T& value, uint64_t index)
		{
			if (!(m_Type && m_Type->IsSZArray()))
				return;

			if (m_Instance)
			{
				using ValueArray = Coral::Array<T>;
				auto arr = m_Instance->GetFieldValue<ValueArray>(m_Name);
				arr[index] = value;
				m_Instance->SetFieldValue(m_Name, arr);
				ValueArray::Free(arr);
			}
			else
			{
				m_ValueBuffer.Write(&value, sizeof(T), index * sizeof(T));
			}
		}

		void Resize(uint64_t newLength)
		{
			uint64_t size = newLength * DataTypeSize(m_DataType);
			uint64_t copySize = std::min<uint64_t>(size, m_ValueBuffer.Size);
			auto oldBuffer = Buffer::Copy(m_ValueBuffer);
			m_ValueBuffer.Allocate(size);
			m_ValueBuffer.ZeroInitialize();
			if (oldBuffer.Data && copySize)
				memcpy(m_ValueBuffer.Data, oldBuffer.Data, (size_t)copySize);
			oldBuffer.Release();
		}

	private:
		std::string m_Name;
		Coral::Type* m_Type = nullptr;
		DataType m_DataType{};
		Buffer m_ValueBuffer;
		Coral::ManagedObject* m_Instance = nullptr;

		friend struct ScriptStorage;
		friend class ScriptEngine;
	};

	struct EntityScriptStorage
	{
		UUID ScriptID{};
		std::unordered_map<uint32_t, FieldStorage> Fields;
		Coral::ManagedObject* Instance = nullptr;
	};

	struct ScriptStorage
	{
		std::unordered_map<UUID, EntityScriptStorage> EntityStorage;

		void InitializeEntityStorage(UUID scriptID, UUID entityID);
		void ShutdownEntityStorage(UUID scriptID, UUID entityID);
		void SynchronizeStorage();
		void CopyTo(ScriptStorage& other) const;
		void CopyEntityStorage(UUID entityID, UUID targetEntityID, ScriptStorage& targetStorage) const;
		void Clear();

		void SetFieldStorage(UUID entityID, uint32_t fieldID, const std::string& name, DataType type, const Buffer& value);

	private:
		void InitializeFieldStorage(EntityScriptStorage& storage, uint32_t fieldID, const FieldMetadata& fieldMetadata);
	};

}
