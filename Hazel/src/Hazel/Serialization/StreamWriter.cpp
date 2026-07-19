#include "hzpch.h"
#include "StreamWriter.h"

namespace Hazel
{
	void StreamWriter::WriteBuffer(Buffer buffer, bool writeSize)
	{
		if (writeSize)
			WriteRaw<uint64_t>(buffer.Size);

		if (buffer.Size > 0)
			WriteData((char*)buffer.Data, (size_t)buffer.Size);
	}

	void StreamWriter::WriteZero(uint64_t size)
	{
		char zero = 0;
		for (uint64_t i = 0; i < size; i++)
			WriteData(&zero, 1);
	}

	void StreamWriter::WriteString(const std::string& string)
	{
		size_t size = string.size();
		WriteData((char*)&size, sizeof(size_t));
		if (!string.empty())
			WriteData(string.data(), string.size());
	}

}
