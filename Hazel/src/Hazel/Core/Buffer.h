#pragma once

#include "Hazel/Core/Core.h"

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

		operator bool() const { return Data != nullptr; }
	};

}
