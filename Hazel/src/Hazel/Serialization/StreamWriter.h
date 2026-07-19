#pragma once

#include "Hazel/Core/Buffer.h"

#include <map>

namespace Hazel
{
	class StreamWriter
	{
	public:
		virtual ~StreamWriter() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t position) = 0;
		virtual bool WriteData(const char* data, size_t size) = 0;

		operator bool() const { return IsStreamGood(); }

		void WriteBuffer(Buffer buffer, bool writeSize = true);
		void WriteZero(uint64_t size);
		void WriteString(const std::string& string);

		template<typename T>
		void WriteRaw(const T& type)
		{
			WriteData((char*)&type, sizeof(T));
		}

		template<typename Key, typename Value>
		void WriteMap(const std::map<Key, Value>& map, bool writeSize = true)
		{
			if (writeSize)
				WriteRaw<uint32_t>((uint32_t)map.size());

			for (const auto& [key, value] : map)
			{
				WriteRaw<Key>(key);
				WriteRaw<Value>(value);
			}
		}
	};

}
