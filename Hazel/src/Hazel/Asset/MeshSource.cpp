#include "hzpch.h"
#include "MeshSource.h"

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	MeshSource::MeshSource(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, const std::vector<glm::vec2>& texCoords)
		: m_Positions(positions), m_TexCoords(texCoords), m_Indices(indices)
	{
		CreateGPUResources();
	}

	void MeshSource::CreateGPUResources()
	{
		if (m_Positions.empty() || m_Indices.empty())
			return;

		const bool hasTexCoords = m_TexCoords.size() == m_Positions.size();

		std::vector<float> flatVertices;
		flatVertices.reserve(m_Positions.size() * (hasTexCoords ? 5 : 3));
		for (size_t i = 0; i < m_Positions.size(); i++)
		{
			const auto& position = m_Positions[i];
			flatVertices.push_back(position.x);
			flatVertices.push_back(position.y);
			flatVertices.push_back(position.z);

			if (hasTexCoords)
			{
				const auto& texCoord = m_TexCoords[i];
				flatVertices.push_back(texCoord.x);
				flatVertices.push_back(texCoord.y);
			}
		}

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(flatVertices.data(), (uint32_t)(flatVertices.size() * sizeof(float)));
		if (hasTexCoords)
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
