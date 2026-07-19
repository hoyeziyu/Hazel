#pragma once

#include "Hazel/Core/Buffer.h"

#include <map>

namespace Hazel
{
	class StreamReader
	{
	public:
		virtual ~StreamReader() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t position) = 0;
		virtual bool ReadData(char* destination, size_t size) = 0;

		operator bool() const { return IsStreamGood(); }

		void ReadBuffer(Buffer& buffer, uint64_t size = 0);
		void ReadString(std::string& string);

		template<typename T>
		void ReadRaw(T& type)
		{
			ReadData((char*)&type, sizeof(T));
		}

		template<typename Key, typename Value>
		void ReadMap(std::map<Key, Value>& map, uint32_t size = 0)
		{
			if (size == 0)
				ReadRaw<uint32_t>(size);

			for (uint32_t i = 0; i < size; i++)
			{
				Key key;
				Value value;
				ReadRaw<Key>(key);
				ReadRaw<Value>(value);
				map[key] = value;
			}
		}
	};

}
