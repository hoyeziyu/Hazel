#pragma once

#include "Hazel/Asset/Asset.h"

#include <cstdint>
#include <filesystem>
#include <unordered_map>

namespace Hazel {

	class AudioCommandRegistry
	{
	public:
		static AudioCommandRegistry& Get();

		void Clear();
		bool LoadFromFile(const std::filesystem::path& path);

		bool TryGetSoundConfig(uint32_t commandId, AssetHandle& outHandle) const;
		bool TryGetSoundConfig(std::string_view debugName, AssetHandle& outHandle) const;

	private:
		std::unordered_map<uint32_t, AssetHandle> m_CommandToSoundConfig;
	};

}
