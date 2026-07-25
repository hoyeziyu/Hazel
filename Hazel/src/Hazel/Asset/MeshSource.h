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
		MeshSource(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
			const std::vector<glm::vec2>& texCoords = {}, const std::vector<glm::vec3>& normals = {});

		AssetType GetAssetType() const override { return AssetType::MeshSource; }

		const Ref<VertexArray>& GetVertexArray();
		const Ref<VertexArray>& GetVertexArray() const { return const_cast<MeshSource*>(this)->GetVertexArray(); }
		const std::vector<glm::vec3>& GetPositions() const { return m_Positions; }
		const std::vector<glm::vec2>& GetTexCoords() const { return m_TexCoords; }
		const std::vector<glm::vec3>& GetNormals() const { return m_Normals; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		static std::vector<glm::vec3> GenerateSmoothNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices);

		void CreateGPUResources();

	private:
		std::vector<glm::vec3> m_Positions;
		std::vector<glm::vec2> m_TexCoords;
		std::vector<glm::vec3> m_Normals;
		std::vector<uint32_t> m_Indices;
		Ref<VertexArray> m_VertexArray;
	};

}
