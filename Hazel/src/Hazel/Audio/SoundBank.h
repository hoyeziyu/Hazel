#pragma once

#include "AudioFileUtils.h"

#include "Hazel/Asset/Asset.h"
#include "Hazel/Core/Buffer.h"
#include "Hazel/Core/Core.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Hazel {

	struct SoundBankFile
	{
		static constexpr uint32_t CurrentVersion = 2;

		struct FileHeader
		{
			char HEADER[4] = { 'H', 'Z', 'S', 'B' };
			uint32_t Version = CurrentVersion;
			uint32_t AudioFileCount = 0;
			uint32_t Padding = 0;
		};

		struct AudioFileInfo
		{
			uint64_t DataOffset = 0;
			uint64_t PackedSize = 0;
			AudioFileUtils::AudioFileInfo Info{};
		};

		FileHeader Header;
		std::unordered_map<AssetHandle, AudioFileInfo> Index;
	};

	class SoundBank
	{
	public:
		SoundBank() = default;
		explicit SoundBank(const std::filesystem::path& path);

		bool IsLoaded() const { return m_Loaded; }
		const std::filesystem::path& GetPath() const { return m_Path; }

		bool Contains(AssetHandle audioHandle) const;
		bool ReadAudioData(AssetHandle audioHandle, Buffer& outBuffer) const;

		static Ref<SoundBank> Create(const std::vector<AssetHandle>& waveAssets, const std::filesystem::path& path);
		static Ref<SoundBank> Load(const std::filesystem::path& path);

	private:
		bool m_Loaded = false;
		SoundBankFile m_File;
		std::filesystem::path m_Path;
	};

}
