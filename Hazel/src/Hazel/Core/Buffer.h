#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"

#include <cstdint>
#include <cstring>

namespace Hazel {

	using byte = uint8_t;

	struct Buffer
	{
		void* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;

		Buffer(const void* data, uint64_t size = 0)
			: Data((void*)data), Size(size) { }

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.Size);
			if (other.Data && other.Size)
				memcpy(buffer.Data, other.Data, (size_t)other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			if (data && size)
				memcpy(buffer.Data, data, (size_t)size);
			return buffer;
		}

		void Allocate(uint64_t size)
		{
			Release();
			Size = size;
			if (size == 0)
				return;
			Data = new byte[size];
		}

		void Release()
		{
			delete[] (byte*)Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, (size_t)Size);
		}

		template<typename T>
		T& Read(uint64_t offset = 0)
		{
			return *(T*)((byte*)Data + offset);
		}

		template<typename T>
		const T& Read(uint64_t offset = 0) const
		{
			return *(T*)((byte*)Data + offset);
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0)
		{
			HZ_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy((byte*)Data + offset, data, (size_t)size);
		}

		operator bool() const { return Data != nullptr; }
	};

}
