#include "hzpch.h"
#include "StaticMesh.h"

namespace Hazel {

	StaticMesh::StaticMesh(AssetHandle meshSource)
		: m_MeshSource(meshSource)
	{
	}

	Ref<StaticMesh> StaticMesh::Create(AssetHandle meshSource)
	{
		return CreateRef<StaticMesh>(meshSource);
	}

}
