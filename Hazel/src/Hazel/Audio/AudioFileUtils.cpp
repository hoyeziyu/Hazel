#include "hzpch.h"
#include "AudioFileUtils.h"

namespace Hazel::AudioFileUtils {

	std::optional<AudioFileInfo> GetFileInfo(const std::filesystem::path& filepath)
	{
		std::error_code ec;
		if (!std::filesystem::exists(filepath, ec))
			return std::nullopt;

		const uint64_t fileSize = std::filesystem::file_size(filepath, ec);
		if (ec || fileSize < 44)
			return std::nullopt;

		AudioFileInfo info{};
		info.FileSize = fileSize;

		std::ifstream stream(filepath, std::ios::binary);
		if (!stream)
			return info;

		char riff[4]{};
		stream.read(riff, 4);
		if (std::memcmp(riff, "RIFF", 4) != 0)
			return info;

		stream.seekg(22, std::ios::beg);
		stream.read(reinterpret_cast<char*>(&info.NumChannels), sizeof(uint16_t));
		stream.read(reinterpret_cast<char*>(&info.SamplingRate), sizeof(uint32_t));
		stream.seekg(34, std::ios::beg);
		stream.read(reinterpret_cast<char*>(&info.BitDepth), sizeof(uint16_t));

		if (info.SamplingRate > 0 && info.NumChannels > 0 && info.BitDepth > 0)
		{
			const uint64_t dataBytes = fileSize > 44 ? fileSize - 44 : 0;
			const uint64_t bytesPerSample = ((uint64_t)info.NumChannels * info.BitDepth) / 8;
			if (bytesPerSample > 0)
				info.Duration = (double)(dataBytes / bytesPerSample) / (double)info.SamplingRate;
		}

		return info;
	}

}
