#include "hzpch.h"
#include "AssimpMeshImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Hazel {

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

		HZ_CORE_INFO("Assimp imported {} vertices, {} indices from {}", positions.size(), indices.size(), m_Path.string());
		return CreateRef<MeshSource>(positions, indices, texCoords, normals);
	}

}
