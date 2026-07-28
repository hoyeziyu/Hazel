#pragma once

#include "Asset.h"

namespace Hazel {

	class AudioFile : public Asset
	{
	public:
		uint64_t FileSize = 0;

		AssetType GetAssetType() const override { return AssetType::Audio; }
	};

}
