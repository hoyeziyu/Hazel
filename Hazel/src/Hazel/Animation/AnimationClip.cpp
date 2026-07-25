#include "hzpch.h"
#include "AnimationClip.h"

#include "Hazel/Core/Log.h"

namespace Hazel {

	AnimationClip::AnimationClip(std::string name, float durationSeconds, std::vector<BoneChannel> channels)
		: m_Name(std::move(name)), m_DurationSeconds(durationSeconds), m_Channels(std::move(channels))
	{
	}

	glm::vec3 AnimationClip::SampleVec3(const std::vector<KeyFrameVec3>& keys, float time)
	{
		if (keys.empty())
			return glm::vec3(0.0f);
		if (keys.size() == 1 || time <= keys.front().Time)
			return keys.front().Value;
		if (time >= keys.back().Time)
			return keys.back().Value;

		for (size_t i = 1; i < keys.size(); ++i)
		{
			if (keys[i].Time >= time)
			{
				const float alpha = (time - keys[i - 1].Time) / (keys[i].Time - keys[i - 1].Time);
				return glm::mix(keys[i - 1].Value, keys[i].Value, alpha);
			}
		}

		return keys.back().Value;
	}

	glm::quat AnimationClip::SampleQuat(const std::vector<KeyFrameQuat>& keys, float time)
	{
		if (keys.empty())
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		if (keys.size() == 1 || time <= keys.front().Time)
			return keys.front().Value;
		if (time >= keys.back().Time)
			return keys.back().Value;

		for (size_t i = 1; i < keys.size(); ++i)
		{
			if (keys[i].Time >= time)
			{
				const float alpha = (time - keys[i - 1].Time) / (keys[i].Time - keys[i - 1].Time);
				return glm::normalize(glm::slerp(keys[i - 1].Value, keys[i].Value, alpha));
			}
		}

		return keys.back().Value;
	}

	AnimationPose AnimationClip::Sample(float normalizedTime, const Skeleton& skeleton) const
	{
		const float time = glm::clamp(normalizedTime, 0.0f, 1.0f);
		AnimationPose pose;
		pose.DurationSeconds = m_DurationSeconds;
		pose.BonePoses.resize(skeleton.GetNumBones());

		for (uint32_t boneIndex = 0; boneIndex < skeleton.GetNumBones(); ++boneIndex)
		{
			const uint32_t channelIndex = boneIndex + 1;
			if (channelIndex < m_Channels.size())
			{
				const BoneChannel& channel = m_Channels[channelIndex];
				pose.BonePoses[boneIndex].Translation = SampleVec3(channel.Translations, time);
				pose.BonePoses[boneIndex].Rotation = SampleQuat(channel.Rotations, time);
				pose.BonePoses[boneIndex].Scale = SampleVec3(channel.Scales, time);
			}
			else
			{
				pose.BonePoses[boneIndex].Translation = skeleton.GetBoneTranslations()[boneIndex];
				pose.BonePoses[boneIndex].Rotation = skeleton.GetBoneRotations()[boneIndex];
				pose.BonePoses[boneIndex].Scale = skeleton.GetBoneScales()[boneIndex];
			}
		}

		return pose;
	}

}
