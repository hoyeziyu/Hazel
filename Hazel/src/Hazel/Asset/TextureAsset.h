#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	class TextureAsset : public Asset
	{
	public:
		Ref<Texture2D> Texture;

		AssetType GetAssetType() const override { return AssetType::Texture; }
	};

}
