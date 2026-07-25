#include "hzpch.h"
#include "MeshSource.h"

#include "AssimpMeshImporter.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Renderer/Buffer.h"

#include <filesystem>

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

	void BoneInfluence::AddBoneData(uint32_t boneInfoIndex, float weight)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (Weights[i] <= 0.0001f)
			{
				BoneInfoIndices[i] = boneInfoIndex;
				Weights[i] = weight;
				return;
			}
		}
	}

	void BoneInfluence::NormalizeWeights()
	{
		float sum = Weights[0] + Weights[1] + Weights[2] + Weights[3];
		if (sum <= 0.0001f)
			return;

		const float inv = 1.0f / sum;
		for (int i = 0; i < 4; ++i)
			Weights[i] *= inv;
	}

	const AnimationClip* MeshSource::GetAnimation(uint32_t index) const
	{
		if (index >= m_Animations.size())
			return nullptr;
		return &m_Animations[index];
	}

	const AnimationClip* MeshSource::GetAnimation(std::string_view name) const
	{
		for (const auto& animation : m_Animations)
		{
			if (animation.GetName() == name)
				return &animation;
		}
		return nullptr;
	}

	const Ref<VertexArray>& MeshSource::GetVertexArray()
	{
		if (!m_VertexArray)
			CreateGPUResources();
		return m_VertexArray;
	}

	const Ref<VertexArray>& MeshSource::GetSkinnedVertexArray()
	{
		if (!m_SkinnedVertexArray)
			CreateSkinnedGPUResources();
		return m_SkinnedVertexArray;
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

	void MeshSource::CreateSkinnedGPUResources()
	{
		if (m_Positions.empty() || m_Indices.empty())
			return;

		if (m_BoneInfluences.size() != m_Positions.size())
			m_BoneInfluences.assign(m_Positions.size(), {});

		const bool hasNormals = m_Normals.size() == m_Positions.size();
		const bool hasTexCoords = m_TexCoords.size() == m_Positions.size();

		std::vector<float> flatVertices;
		flatVertices.reserve(m_Positions.size() * 16);
		for (size_t i = 0; i < m_Positions.size(); i++)
		{
			const auto& position = m_Positions[i];
			flatVertices.push_back(position.x);
			flatVertices.push_back(position.y);
			flatVertices.push_back(position.z);

			const auto& normal = hasNormals ? m_Normals[i] : glm::vec3(0.0f, 1.0f, 0.0f);
			flatVertices.push_back(normal.x);
			flatVertices.push_back(normal.y);
			flatVertices.push_back(normal.z);

			const auto& texCoord = hasTexCoords ? m_TexCoords[i] : glm::vec2(0.0f);
			flatVertices.push_back(texCoord.x);
			flatVertices.push_back(texCoord.y);

			const auto& influence = m_BoneInfluences[i];
			flatVertices.push_back((float)influence.BoneInfoIndices[0]);
			flatVertices.push_back((float)influence.BoneInfoIndices[1]);
			flatVertices.push_back((float)influence.BoneInfoIndices[2]);
			flatVertices.push_back((float)influence.BoneInfoIndices[3]);

			flatVertices.push_back(influence.Weights[0]);
			flatVertices.push_back(influence.Weights[1]);
			flatVertices.push_back(influence.Weights[2]);
			flatVertices.push_back(influence.Weights[3]);
		}

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(flatVertices.data(), (uint32_t)(flatVertices.size() * sizeof(float)));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float4, "a_BoneIndices" },
			{ ShaderDataType::Float4, "a_BoneWeights" }
		});

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)m_Indices.size());

		m_SkinnedVertexArray = VertexArray::Create();
		m_SkinnedVertexArray->AddVertexBuffer(vertexBuffer);
		m_SkinnedVertexArray->SetIndexBuffer(indexBuffer);
	}

	void MeshSource::EnsureRigDataFromSource()
	{
		if (!m_SourceModelPath.empty() && !HasSkeleton())
		{
			std::filesystem::path path = m_SourceModelPath;
			if (auto project = Project::GetActive())
			{
				if (!path.is_absolute())
					path = project->GetAssetPath(m_SourceModelPath);
			}

			if (std::filesystem::exists(path))
			{
				AssimpMeshImporter importer(path);
				if (Ref<MeshSource> imported = importer.ImportToMeshSource())
				{
					m_Skeleton = imported->GetSkeleton();
					m_BoneInfo = imported->GetBoneInfo();
					m_BoneInfluences = imported->GetBoneInfluences();
					m_Animations = imported->GetAnimations();
					m_SkinnedVertexArray.reset();
				}
			}
		}
	}

}
