#pragma once

#include "Hazel/Asset/Asset.h"
#include "Hazel/Animation/Skeleton.h"
#include "Hazel/Animation/AnimationClip.h"
#include "Hazel/Renderer/VertexArray.h"

#include <glm/glm.hpp>
#include <vector>

namespace Hazel {

	struct BoneInfo
	{
		glm::mat4 InverseBindPose{ 1.0f };
		uint32_t BoneIndex = Skeleton::NullIndex;
	};

	struct BoneInfluence
	{
		uint32_t BoneInfoIndices[4]{ 0, 0, 0, 0 };
		float Weights[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t boneInfoIndex, float weight);
		void NormalizeWeights();
	};

	class MeshSource : public Asset
	{
	public:
		MeshSource() = default;
		MeshSource(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
			const std::vector<glm::vec2>& texCoords = {}, const std::vector<glm::vec3>& normals = {});

		AssetType GetAssetType() const override { return AssetType::MeshSource; }

		const Ref<VertexArray>& GetVertexArray();
		const Ref<VertexArray>& GetVertexArray() const { return const_cast<MeshSource*>(this)->GetVertexArray(); }

		const Ref<VertexArray>& GetSkinnedVertexArray();
		const Ref<VertexArray>& GetSkinnedVertexArray() const { return const_cast<MeshSource*>(this)->GetSkinnedVertexArray(); }

		const std::vector<glm::vec3>& GetPositions() const { return m_Positions; }
		const std::vector<glm::vec2>& GetTexCoords() const { return m_TexCoords; }
		const std::vector<glm::vec3>& GetNormals() const { return m_Normals; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		bool HasSkeleton() const { return m_Skeleton && m_Skeleton->GetNumBones() > 0; }
		const Ref<Skeleton>& GetSkeleton() const { return m_Skeleton; }
		Ref<Skeleton>& GetSkeleton() { return m_Skeleton; }

		const std::vector<BoneInfo>& GetBoneInfo() const { return m_BoneInfo; }
		std::vector<BoneInfo>& GetBoneInfo() { return m_BoneInfo; }

		const std::vector<BoneInfluence>& GetBoneInfluences() const { return m_BoneInfluences; }
		std::vector<BoneInfluence>& GetBoneInfluences() { return m_BoneInfluences; }

		bool IsRigged() const { return HasSkeleton() && !m_BoneInfluences.empty(); }

		uint32_t GetAnimationCount() const { return static_cast<uint32_t>(m_Animations.size()); }
		const AnimationClip* GetAnimation(uint32_t index) const;
		const AnimationClip* GetAnimation(std::string_view name) const;
		const std::vector<AnimationClip>& GetAnimations() const { return m_Animations; }
		std::vector<AnimationClip>& GetAnimations() { return m_Animations; }

		static std::vector<glm::vec3> GenerateSmoothNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices);

		void CreateGPUResources();

		void EnsureRigDataFromSource();

		void SetSourceModelPath(const std::string& path) { m_SourceModelPath = path; }
		const std::string& GetSourceModelPath() const { return m_SourceModelPath; }

	private:
		void CreateSkinnedGPUResources();

		std::vector<glm::vec3> m_Positions;
		std::vector<glm::vec2> m_TexCoords;
		std::vector<glm::vec3> m_Normals;
		std::vector<uint32_t> m_Indices;

		Ref<Skeleton> m_Skeleton;
		std::vector<BoneInfo> m_BoneInfo;
		std::vector<BoneInfluence> m_BoneInfluences;
		std::vector<AnimationClip> m_Animations;

		Ref<VertexArray> m_VertexArray;
		Ref<VertexArray> m_SkinnedVertexArray;
		std::string m_SourceModelPath;
	};

}
