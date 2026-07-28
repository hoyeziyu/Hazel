#pragma once

#include "Asset.h"

namespace Hazel {

	class SoundConfigAsset : public Asset
	{
	public:
		AssetHandle DataSourceAsset = 0;
		bool IsLooping = false;
		float VolumeMultiplier = 1.0f;
		float PitchMultiplier = 1.0f;

		AssetType GetAssetType() const override { return AssetType::SoundConfig; }
	};

}
