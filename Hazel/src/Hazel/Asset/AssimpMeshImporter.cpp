#include "hzpch.h"
#include "AssimpMeshImporter.h"

#include "AssimpAnimationImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Hazel {

	namespace {

		glm::mat4 Mat4FromAIMatrix4x4(const aiMatrix4x4& matrix)
		{
			glm::mat4 result;
			result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
			result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
			result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
			result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
			return result;
		}

	}

	AssimpMeshImporter::AssimpMeshImporter(const std::filesystem::path& path)
		: m_Path(path)
	{
	}

	Ref<MeshSource> AssimpMeshImporter::ImportToMeshSource() const
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(m_Path.string(),
			aiProcess_Triangulate
			| aiProcess_GenNormals
			| aiProcess_GenUVCoords
			| aiProcess_JoinIdenticalVertices
			| aiProcess_ImproveCacheLocality
			| aiProcess_LimitBoneWeights
			| aiProcess_GlobalScale);

		if (!scene || !scene->HasMeshes())
		{
			HZ_CORE_ERROR("Assimp failed to import mesh: {}", m_Path.string());
			if (const char* error = importer.GetErrorString())
				HZ_CORE_ERROR("Assimp error: {}", error);
			return nullptr;
		}

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> texCoords;
		std::vector<glm::vec3> normals;
		std::vector<uint32_t> indices;

		uint32_t vertexOffset = 0;
		for (unsigned meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];
			if (!mesh)
				continue;

			for (unsigned i = 0; i < mesh->mNumVertices; i++)
			{
				const aiVector3D& v = mesh->mVertices[i];
				positions.emplace_back(v.x, v.y, v.z);

				if (mesh->HasNormals())
				{
					const aiVector3D& n = mesh->mNormals[i];
					normals.emplace_back(n.x, n.y, n.z);
				}
				else
				{
					normals.emplace_back(0.0f, 1.0f, 0.0f);
				}

				if (mesh->HasTextureCoords(0))
				{
					const aiVector3D& uv = mesh->mTextureCoords[0][i];
					texCoords.emplace_back(uv.x, uv.y);
				}
				else
				{
					texCoords.emplace_back(0.0f, 0.0f);
				}
			}

			for (unsigned faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
			{
				const aiFace& face = mesh->mFaces[faceIndex];
				for (unsigned j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j] + vertexOffset);
			}

			vertexOffset += mesh->mNumVertices;
		}

		if (positions.empty() || indices.empty())
			return nullptr;

		Ref<MeshSource> meshSource = CreateRef<MeshSource>(positions, indices, texCoords, normals);

		if (Scope<Skeleton> skeleton = AssimpAnimationImporter::ImportSkeleton(scene))
		{
			meshSource->GetSkeleton() = CreateRef<Skeleton>(*skeleton);
			meshSource->GetBoneInfluences().resize(positions.size());

			for (unsigned meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
			{
				const aiMesh* mesh = scene->mMeshes[meshIndex];
				if (!mesh || mesh->mNumBones == 0)
					continue;

				uint32_t baseVertex = 0;
				for (unsigned i = 0; i < meshIndex; ++i)
					baseVertex += scene->mMeshes[i]->mNumVertices;

				for (unsigned boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
				{
					const aiBone* bone = mesh->mBones[boneIndex];
					bool hasWeight = false;
					for (unsigned j = 0; j < bone->mNumWeights; ++j)
					{
						if (bone->mWeights[j].mWeight > 0.000001f)
							hasWeight = true;
					}
					if (!hasWeight)
						continue;

					const uint32_t skeletonBoneIndex = meshSource->GetSkeleton()->GetBoneIndex(bone->mName.C_Str());
					if (skeletonBoneIndex == Skeleton::NullIndex)
					{
						HZ_CORE_WARN("Could not find mesh bone '{}' in skeleton", bone->mName.C_Str());
						continue;
					}

					uint32_t boneInfoIndex = ~0u;
					for (size_t j = 0; j < meshSource->GetBoneInfo().size(); ++j)
					{
						if (meshSource->GetBoneInfo()[j].BoneIndex == skeletonBoneIndex)
						{
							boneInfoIndex = static_cast<uint32_t>(j);
							break;
						}
					}

					if (boneInfoIndex == ~0u)
					{
						boneInfoIndex = static_cast<uint32_t>(meshSource->GetBoneInfo().size());
						meshSource->GetBoneInfo().push_back({ Mat4FromAIMatrix4x4(bone->mOffsetMatrix), skeletonBoneIndex });
					}

					for (unsigned j = 0; j < bone->mNumWeights; ++j)
					{
						const int vertexId = static_cast<int>(baseVertex + bone->mWeights[j].mVertexId);
						const float weight = bone->mWeights[j].mWeight;
						meshSource->GetBoneInfluences()[vertexId].AddBoneData(boneInfoIndex, weight);
					}
				}
			}

			for (auto& influence : meshSource->GetBoneInfluences())
				influence.NormalizeWeights();

			for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex)
			{
				if (Scope<AnimationClip> clip = AssimpAnimationImporter::ImportAnimation(scene, animationIndex, *meshSource->GetSkeleton()))
					meshSource->GetAnimations().push_back(std::move(*clip));
			}

			HZ_CORE_INFO("Assimp imported rigged mesh: {} vertices, {} bones, {} animations from {}",
				positions.size(), meshSource->GetSkeleton()->GetNumBones(), meshSource->GetAnimationCount(), m_Path.string());
		}
		else
		{
			HZ_CORE_INFO("Assimp imported {} vertices, {} indices from {}", positions.size(), indices.size(), m_Path.string());
		}

		return meshSource;
	}

}
