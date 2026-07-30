#include "hzpch.h"
#include "AudioCommandRegistry.h"

#include "Hazel/Core/Hash.h"

#include <fstream>
#include <sstream>

namespace Hazel {

	namespace {

		std::string Trim(std::string value)
		{
			while (!value.empty() && std::isspace((unsigned char)value.front()))
				value.erase(value.begin());
			while (!value.empty() && std::isspace((unsigned char)value.back()))
				value.pop_back();
			return value;
		}

		std::string Unquote(std::string value)
		{
			value = Trim(value);
			if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
				return value.substr(1, value.size() - 2);
			return value;
		}

		uint64_t ParseUint64Field(const std::string& yamlString, std::string_view key)
		{
			const std::string needle = std::string(key) + ":";
			size_t pos = yamlString.find(needle);
			if (pos == std::string::npos)
				return 0;

			size_t start = pos + needle.size();
			while (start < yamlString.size() && std::isspace((unsigned char)yamlString[start]))
				start++;

			size_t end = start;
			while (end < yamlString.size() && std::isdigit((unsigned char)yamlString[end]))
				end++;

			if (end <= start)
				return 0;

			return (uint64_t)std::strtoull(yamlString.substr(start, end - start).c_str(), nullptr, 10);
		}

	}

	AudioCommandRegistry& AudioCommandRegistry::Get()
	{
		static AudioCommandRegistry instance;
		return instance;
	}

	void AudioCommandRegistry::Clear()
	{
		m_CommandToSoundConfig.clear();
	}

	bool AudioCommandRegistry::LoadFromFile(const std::filesystem::path& path)
	{
		Clear();

		std::error_code ec;
		if (!std::filesystem::exists(path, ec))
		{
			HZ_CORE_WARN("[Audio] AudioCommandsRegistry not found: {}", path.string());
			return false;
		}

		std::ifstream stream(path);
		if (!stream)
			return false;

		std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

		std::string currentDebugName;
		std::string currentBlock;
		bool inTrigger = false;

		size_t start = 0;
		while (start < contents.size())
		{
			size_t end = contents.find('\n', start);
			if (end == std::string::npos)
				end = contents.size();

			std::string line = contents.substr(start, end - start);
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			start = (end == contents.size()) ? contents.size() + 1 : end + 1;

			const std::string trimmed = Trim(line);
			if (trimmed.empty())
				continue;

			if (trimmed.rfind("- DebugName:", 0) == 0)
			{
				if (inTrigger && !currentDebugName.empty())
				{
					const uint64_t target = ParseUint64Field(currentBlock, "Target");
					if (target != 0)
						m_CommandToSoundConfig[Hash::GenerateFNVHash(currentDebugName)] = AssetHandle(target);
				}

				currentDebugName = Unquote(trimmed.substr(std::string("- DebugName:").size()));
				currentBlock.clear();
				inTrigger = true;
				continue;
			}

			if (inTrigger)
				currentBlock += line + "\n";
		}

		if (inTrigger && !currentDebugName.empty())
		{
			const uint64_t target = ParseUint64Field(currentBlock, "Target");
			if (target != 0)
				m_CommandToSoundConfig[Hash::GenerateFNVHash(currentDebugName)] = AssetHandle(target);
		}

		HZ_CORE_INFO("[Audio] Loaded {} audio command triggers from {}", m_CommandToSoundConfig.size(), path.string());
		return !m_CommandToSoundConfig.empty();
	}

	bool AudioCommandRegistry::TryGetSoundConfig(uint32_t commandId, AssetHandle& outHandle) const
	{
		auto it = m_CommandToSoundConfig.find(commandId);
		if (it == m_CommandToSoundConfig.end())
			return false;

		outHandle = it->second;
		return (uint64_t)outHandle != 0;
	}

	bool AudioCommandRegistry::TryGetSoundConfig(std::string_view debugName, AssetHandle& outHandle) const
	{
		return TryGetSoundConfig(Hash::GenerateFNVHash(debugName), outHandle);
	}

}
