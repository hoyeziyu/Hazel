#pragma once

#include "AssetPackFile.h"

#include <atomic>
#include <filesystem>

namespace Hazel {

	struct Buffer;

	class AssetPackSerializer
	{
	public:
		static void Serialize(const std::filesystem::path& path, AssetPackFile& file, Buffer appBinary, std::atomic<float>& progress);
		static bool DeserializeIndex(const std::filesystem::path& path, AssetPackFile& file);

		static uint64_t CalculateIndexTableSize(const AssetPackFile& file);
	};

}
