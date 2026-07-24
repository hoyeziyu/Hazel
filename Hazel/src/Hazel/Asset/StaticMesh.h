#pragma once

#include "Hazel/Asset/Asset.h"

namespace Hazel {

	class StaticMesh : public Asset
	{
	public:
		StaticMesh() = default;
		explicit StaticMesh(AssetHandle meshSource);

		AssetType GetAssetType() const override { return AssetType::StaticMesh; }

		AssetHandle GetMeshSource() const { return m_MeshSource; }

		static Ref<StaticMesh> Create(AssetHandle meshSource);

	private:
		AssetHandle m_MeshSource = 0;
	};

}
