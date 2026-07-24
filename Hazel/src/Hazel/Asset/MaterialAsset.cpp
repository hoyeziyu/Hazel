#include "hzpch.h"
#include "MaterialAsset.h"

#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/TextureAsset.h"

namespace Hazel {

	Ref<Texture2D> MaterialAsset::GetAlbedoTexture() const
	{
		if (!AlbedoMap || !AssetManager::IsAssetHandleValid(AlbedoMap))
			return nullptr;

		if (auto textureAsset = AssetManager::GetAsset<TextureAsset>(AlbedoMap))
			return textureAsset->Texture;

		return nullptr;
	}

	bool MaterialAsset::HasAlbedoMap() const
	{
		return GetAlbedoTexture() != nullptr;
	}

	Ref<MaterialAsset> MaterialAsset::Create()
	{
		return CreateRef<MaterialAsset>();
	}

}
