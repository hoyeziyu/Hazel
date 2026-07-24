#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Renderer/VertexArray.h"

#include <glm/glm.hpp>
#include <vector>

namespace Hazel {

	class MeshSource : public Asset
	{
	public:
		MeshSource() = default;
		MeshSource(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices);

		AssetType GetAssetType() const override { return AssetType::MeshSource; }

		const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const std::vector<glm::vec3>& GetPositions() const { return m_Positions; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		void CreateGPUResources();

	private:
		std::vector<glm::vec3> m_Positions;
		std::vector<uint32_t> m_Indices;
		Ref<VertexArray> m_VertexArray;
	};

}
