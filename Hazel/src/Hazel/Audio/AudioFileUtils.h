#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

namespace Hazel::AudioFileUtils {

	struct AudioFileInfo
	{
		double Duration = 0.0;
		uint32_t SamplingRate = 44100;
		uint16_t BitDepth = 16;
		uint16_t NumChannels = 1;
		uint64_t FileSize = 0;
	};

	std::optional<AudioFileInfo> GetFileInfo(const std::filesystem::path& filepath);

}
