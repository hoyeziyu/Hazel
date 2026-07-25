#include "hzpch.h"
#include "Skeleton.h"

#include "Hazel/Core/Log.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Hazel {

	Skeleton::Skeleton(uint32_t size)
	{
		m_BoneNames.reserve(size);
		m_ParentBoneIndices.reserve(size);
		m_BoneTranslations.reserve(size);
		m_BoneRotations.reserve(size);
		m_BoneScales.reserve(size);
	}

	uint32_t Skeleton::AddBone(const std::string& name, uint32_t parentIndex, const glm::mat4& transform)
	{
		const uint32_t index = static_cast<uint32_t>(m_BoneNames.size());
		m_BoneNames.emplace_back(name);
		m_ParentBoneIndices.emplace_back(parentIndex);
		m_BoneTranslations.emplace_back();
		m_BoneRotations.emplace_back(1.0f, 0.0f, 0.0f, 0.0f);
		m_BoneScales.emplace_back(1.0f);

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, m_BoneScales.back(), m_BoneRotations.back(), m_BoneTranslations.back(), skew, perspective);

		return index;
	}

	uint32_t Skeleton::GetBoneIndex(std::string_view name) const
	{
		for (size_t i = 0; i < m_BoneNames.size(); ++i)
		{
			if (m_BoneNames[i] == name)
				return static_cast<uint32_t>(i);
		}
		return NullIndex;
	}

	const std::string& Skeleton::GetBoneName(uint32_t boneIndex) const
	{
		HZ_CORE_ASSERT(boneIndex < m_BoneNames.size(), "Skeleton::GetBoneName out of range");
		return m_BoneNames[boneIndex];
	}

	uint32_t Skeleton::GetParentBoneIndex(uint32_t boneIndex) const
	{
		HZ_CORE_ASSERT(boneIndex < m_ParentBoneIndices.size(), "Skeleton::GetParentBoneIndex out of range");
		return m_ParentBoneIndices[boneIndex];
	}

	void Skeleton::SetBones(std::vector<std::string> boneNames,
		std::vector<uint32_t> parentBoneIndices,
		std::vector<glm::vec3> boneTranslations,
		std::vector<glm::quat> boneRotations,
		std::vector<glm::vec3> boneScales)
	{
		HZ_CORE_ASSERT(parentBoneIndices.size() == boneNames.size(), "Skeleton::SetBones size mismatch");
		HZ_CORE_ASSERT(boneTranslations.size() == boneNames.size(), "Skeleton::SetBones size mismatch");
		HZ_CORE_ASSERT(boneRotations.size() == boneNames.size(), "Skeleton::SetBones size mismatch");
		HZ_CORE_ASSERT(boneScales.size() == boneNames.size(), "Skeleton::SetBones size mismatch");

		m_BoneNames = std::move(boneNames);
		m_ParentBoneIndices = std::move(parentBoneIndices);
		m_BoneTranslations = std::move(boneTranslations);
		m_BoneRotations = std::move(boneRotations);
		m_BoneScales = std::move(boneScales);
	}

}
