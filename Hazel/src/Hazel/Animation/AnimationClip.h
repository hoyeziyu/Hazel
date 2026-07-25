#pragma once

#include "Skeleton.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

namespace Hazel {

	struct BonePose
	{
		glm::vec3 Translation{ 0.0f };
		glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f };
	};

	struct AnimationPose
	{
		std::vector<BonePose> BonePoses;
		float DurationSeconds = 1.0f;
	};

	class AnimationClip
	{
	public:
		static constexpr uint32_t MaxBones = 100;

		struct KeyFrameVec3
		{
			float Time = 0.0f;
			glm::vec3 Value{ 0.0f };
		};

		struct KeyFrameQuat
		{
			float Time = 0.0f;
			glm::quat Value{ 1.0f, 0.0f, 0.0f, 0.0f };
		};

		struct BoneChannel
		{
			std::vector<KeyFrameVec3> Translations;
			std::vector<KeyFrameQuat> Rotations;
			std::vector<KeyFrameVec3> Scales;
		};

		AnimationClip() = default;
		AnimationClip(std::string name, float durationSeconds, std::vector<BoneChannel> channels);

		const std::string& GetName() const { return m_Name; }
		float GetDurationSeconds() const { return m_DurationSeconds; }
		uint32_t GetNumChannels() const { return static_cast<uint32_t>(m_Channels.size()); }
		const std::vector<BoneChannel>& GetChannels() const { return m_Channels; }

		AnimationPose Sample(float normalizedTime, const Skeleton& skeleton) const;

	private:
		static glm::vec3 SampleVec3(const std::vector<KeyFrameVec3>& keys, float time);
		static glm::quat SampleQuat(const std::vector<KeyFrameQuat>& keys, float time);

		std::string m_Name;
		float m_DurationSeconds = 1.0f;
		std::vector<BoneChannel> m_Channels;
	};

}
