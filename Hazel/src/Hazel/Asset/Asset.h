#pragma once

#include "Hazel/Core/UUID.h"
#include "AssetTypes.h"

namespace Hazel {

	using AssetHandle = UUID;

	class Asset
	{
	public:
		AssetHandle Handle = 0;
		virtual ~Asset() = default;

		virtual AssetType GetAssetType() const { return AssetType::None; }
	};

}
