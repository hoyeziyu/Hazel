#include "hzpch.h"
#include "StreamReader.h"

namespace Hazel
{
	void StreamReader::ReadBuffer(Buffer& buffer, uint64_t size)
	{
		if (size == 0)
			ReadRaw<uint64_t>(size);

		buffer.Allocate(size);
		if (size > 0)
			ReadData((char*)buffer.Data, (size_t)size);
	}

	void StreamReader::ReadString(std::string& string)
	{
		size_t size = 0;
		ReadData((char*)&size, sizeof(size_t));
		string.resize(size);
		if (size > 0)
			ReadData(string.data(), size);
	}

}
