#pragma once

#include "StreamWriter.h"
#include "StreamReader.h"

#include <filesystem>
#include <fstream>

namespace Hazel
{
	class FileStreamWriter : public StreamWriter
	{
	public:
		FileStreamWriter(const std::filesystem::path& path);
		FileStreamWriter(const FileStreamWriter&) = delete;
		~FileStreamWriter();

		bool IsStreamGood() const override { return m_Stream.good(); }
		uint64_t GetStreamPosition() override { return (uint64_t)m_Stream.tellp(); }
		void SetStreamPosition(uint64_t position) override { m_Stream.seekp((std::streampos)position); }
		bool WriteData(const char* data, size_t size) override;

	private:
		std::filesystem::path m_Path;
		std::ofstream m_Stream;
	};

	class FileStreamReader : public StreamReader
	{
	public:
		FileStreamReader(const std::filesystem::path& path);
		FileStreamReader(const FileStreamReader&) = delete;
		~FileStreamReader();

		bool IsStreamGood() const override { return m_Stream.good(); }
		uint64_t GetStreamPosition() override { return (uint64_t)m_Stream.tellg(); }
		void SetStreamPosition(uint64_t position) override { m_Stream.seekg((std::streampos)position); }
		bool ReadData(char* destination, size_t size) override;

	private:
		std::filesystem::path m_Path;
		std::ifstream m_Stream;
	};

}
