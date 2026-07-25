#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Hazel {

	class Skeleton
	{
	public:
		static constexpr uint32_t NullIndex = ~0u;

		Skeleton() = default;
		explicit Skeleton(uint32_t size);

		const glm::mat4& GetTransform() const { return m_Transform; }
		void SetTransform(const glm::mat4& transform) { m_Transform = transform; }

		uint32_t AddBone(const std::string& name, uint32_t parentIndex, const glm::mat4& transform);
		uint32_t GetBoneIndex(std::string_view name) const;

		uint32_t GetNumBones() const { return static_cast<uint32_t>(m_BoneNames.size()); }
		const std::string& GetBoneName(uint32_t boneIndex) const;
		uint32_t GetParentBoneIndex(uint32_t boneIndex) const;

		const std::vector<std::string>& GetBoneNames() const { return m_BoneNames; }
		const std::vector<uint32_t>& GetParentBoneIndices() const { return m_ParentBoneIndices; }
		const std::vector<glm::vec3>& GetBoneTranslations() const { return m_BoneTranslations; }
		const std::vector<glm::quat>& GetBoneRotations() const { return m_BoneRotations; }
		const std::vector<glm::vec3>& GetBoneScales() const { return m_BoneScales; }

		void SetBones(std::vector<std::string> boneNames,
			std::vector<uint32_t> parentBoneIndices,
			std::vector<glm::vec3> boneTranslations,
			std::vector<glm::quat> boneRotations,
			std::vector<glm::vec3> boneScales);

	private:
		std::vector<std::string> m_BoneNames;
		std::vector<uint32_t> m_ParentBoneIndices;
		std::vector<glm::vec3> m_BoneTranslations;
		std::vector<glm::quat> m_BoneRotations;
		std::vector<glm::vec3> m_BoneScales;
		glm::mat4 m_Transform = glm::mat4(1.0f);
	};

}
