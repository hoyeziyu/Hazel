#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace Hazel {

	class MaterialAsset : public Asset
	{
	public:
		MaterialAsset() = default;

		glm::vec3 AlbedoColor{ 1.0f, 1.0f, 1.0f };
		float Metalness = 0.0f;
		float Roughness = 0.5f;
		float Emission = 0.0f;
		AssetHandle AlbedoMap = 0;

		AssetType GetAssetType() const override { return AssetType::Material; }

		Ref<Texture2D> GetAlbedoTexture() const;
		bool HasAlbedoMap() const;

		static Ref<MaterialAsset> Create();
	};

}
