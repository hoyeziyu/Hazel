#include "hzpch.h"
#include "FileStream.h"

namespace Hazel
{
	FileStreamWriter::FileStreamWriter(const std::filesystem::path& path)
		: m_Path(path)
	{
		m_Stream = std::ofstream(path, std::ios::binary | std::ios::trunc);
	}

	FileStreamWriter::~FileStreamWriter()
	{
		if (m_Stream.is_open())
			m_Stream.close();
	}

	bool FileStreamWriter::WriteData(const char* data, size_t size)
	{
		m_Stream.write(data, (std::streamsize)size);
		return m_Stream.good();
	}

	FileStreamReader::FileStreamReader(const std::filesystem::path& path)
		: m_Path(path)
	{
		m_Stream = std::ifstream(path, std::ios::binary);
	}

	FileStreamReader::~FileStreamReader()
	{
		if (m_Stream.is_open())
			m_Stream.close();
	}

	bool FileStreamReader::ReadData(char* destination, size_t size)
	{
		m_Stream.read(destination, (std::streamsize)size);
		return m_Stream.good();
	}

}
