#include "hzpch.h"
#include "MeshSource.h"

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	MeshSource::MeshSource(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
		const std::vector<glm::vec2>& texCoords, const std::vector<glm::vec3>& normals)
		: m_Positions(positions), m_TexCoords(texCoords), m_Normals(normals), m_Indices(indices)
	{
		if (m_Normals.empty() && !m_Positions.empty() && !m_Indices.empty())
			m_Normals = GenerateSmoothNormals(m_Positions, m_Indices);

		if (m_TexCoords.empty() && !m_Positions.empty())
			m_TexCoords.assign(m_Positions.size(), glm::vec2(0.0f));
	}

	const Ref<VertexArray>& MeshSource::GetVertexArray()
	{
		if (!m_VertexArray)
			CreateGPUResources();
		return m_VertexArray;
	}

	std::vector<glm::vec3> MeshSource::GenerateSmoothNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices)
	{
		std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));

		for (size_t i = 0; i + 2 < indices.size(); i += 3)
		{
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			const glm::vec3& p0 = positions[i0];
			const glm::vec3& p1 = positions[i1];
			const glm::vec3& p2 = positions[i2];

			glm::vec3 faceNormal = glm::cross(p1 - p0, p2 - p0);
			normals[i0] += faceNormal;
			normals[i1] += faceNormal;
			normals[i2] += faceNormal;
		}

		for (auto& normal : normals)
		{
			if (glm::dot(normal, normal) > 0.0001f)
				normal = glm::normalize(normal);
			else
				normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}

		return normals;
	}

	void MeshSource::CreateGPUResources()
	{
		if (m_Positions.empty() || m_Indices.empty())
			return;

		const bool hasNormals = m_Normals.size() == m_Positions.size();
		const bool hasTexCoords = m_TexCoords.size() == m_Positions.size();

		std::vector<float> flatVertices;
		flatVertices.reserve(m_Positions.size() * 8);
		for (size_t i = 0; i < m_Positions.size(); i++)
		{
			const auto& position = m_Positions[i];
			flatVertices.push_back(position.x);
			flatVertices.push_back(position.y);
			flatVertices.push_back(position.z);

			if (hasNormals)
			{
				const auto& normal = m_Normals[i];
				flatVertices.push_back(normal.x);
				flatVertices.push_back(normal.y);
				flatVertices.push_back(normal.z);
			}

			if (hasTexCoords)
			{
				const auto& texCoord = m_TexCoords[i];
				flatVertices.push_back(texCoord.x);
				flatVertices.push_back(texCoord.y);
			}
		}

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(flatVertices.data(), (uint32_t)(flatVertices.size() * sizeof(float)));

		if (hasNormals && hasTexCoords)
		{
			vertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});
		}
		else if (hasTexCoords)
		{
			vertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});
		}
		else
		{
			vertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" }
			});
		}

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)m_Indices.size());

		m_VertexArray = VertexArray::Create();
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}

}
