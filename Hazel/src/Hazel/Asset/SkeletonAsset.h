#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Animation/Skeleton.h"

namespace Hazel {

	class SkeletonAsset : public Asset
	{
	public:
		SkeletonAsset() = default;
		explicit SkeletonAsset(AssetHandle meshSource);

		AssetType GetAssetType() const override { return AssetType::Skeleton; }

		AssetHandle GetMeshSource() const { return m_MeshSource; }
		void SetMeshSource(AssetHandle meshSource) { m_MeshSource = meshSource; }

	private:
		AssetHandle m_MeshSource = 0;
	};

}
