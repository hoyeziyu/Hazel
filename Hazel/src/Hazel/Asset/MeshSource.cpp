#include "hzpch.h"
#include "MeshSource.h"

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	MeshSource::MeshSource(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices)
		: m_Positions(positions), m_Indices(indices)
	{
		CreateGPUResources();
	}

	void MeshSource::CreateGPUResources()
	{
		if (m_Positions.empty() || m_Indices.empty())
			return;

		std::vector<float> flatVertices;
		flatVertices.reserve(m_Positions.size() * 3);
		for (const auto& position : m_Positions)
		{
			flatVertices.push_back(position.x);
			flatVertices.push_back(position.y);
			flatVertices.push_back(position.z);
		}

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(flatVertices.data(), (uint32_t)(flatVertices.size() * sizeof(float)));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)m_Indices.size());

		m_VertexArray = VertexArray::Create();
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}

}
