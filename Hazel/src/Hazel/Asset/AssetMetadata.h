#pragma once

#include "Asset.h"

#include <filesystem>

namespace Hazel {

	struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;

		bool IsDataLoaded = false;

		bool IsValid() const { return (uint64_t)Handle != 0 && Type != AssetType::None; }
	};

}
