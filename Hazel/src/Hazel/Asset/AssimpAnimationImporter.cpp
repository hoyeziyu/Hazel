#include "hzpch.h"
#include "AssimpAnimationImporter.h"

#include "Hazel/Core/Log.h"

#include <assimp/scene.h>

#include <map>
#include <set>
#include <unordered_map>

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

		std::string SanitiseAnimationName(std::string_view animationName)
		{
			std::string name(animationName);
			if (auto pos = name.find_last_of('|'); pos != std::string::npos)
				name = name.substr(pos + 1);
			return name;
		}

		class BoneHierarchy
		{
		public:
			explicit BoneHierarchy(const aiScene* scene) : m_Scene(scene) {}

			Scope<Skeleton> CreateSkeleton()
			{
				ExtractBones();
				if (m_Bones.empty())
					return nullptr;

				auto skeleton = CreateScope<Skeleton>(static_cast<uint32_t>(m_Bones.size()));
				TraverseNode(m_Scene->mRootNode, skeleton.get());
				return skeleton;
			}

		private:
			void ExtractBones()
			{
				for (uint32_t meshIndex = 0; meshIndex < m_Scene->mNumMeshes; ++meshIndex)
				{
					const aiMesh* mesh = m_Scene->mMeshes[meshIndex];
					for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
						m_Bones.emplace(mesh->mBones[boneIndex]->mName.C_Str());
				}

				for (uint32_t animationIndex = 0; animationIndex < m_Scene->mNumAnimations; ++animationIndex)
				{
					const aiAnimation* animation = m_Scene->mAnimations[animationIndex];
					for (uint32_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex)
						m_Bones.emplace(animation->mChannels[channelIndex]->mNodeName.C_Str());
				}
			}

			void TraverseNode(aiNode* node, Skeleton* skeleton, const glm::mat4& parentTransform = glm::mat4(1.0f))
			{
				if (m_Bones.find(node->mName.C_Str()) != m_Bones.end())
				{
					skeleton->SetTransform(parentTransform);
					TraverseBone(node, skeleton, Skeleton::NullIndex);
				}
				else
				{
					const glm::mat4 transform = parentTransform * Mat4FromAIMatrix4x4(node->mTransformation);
					for (uint32_t nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex)
						TraverseNode(node->mChildren[nodeIndex], skeleton, transform);
				}
			}

			void TraverseBone(aiNode* node, Skeleton* skeleton, uint32_t parentIndex)
			{
				const uint32_t boneIndex = skeleton->AddBone(node->mName.C_Str(), parentIndex, Mat4FromAIMatrix4x4(node->mTransformation));
				for (uint32_t nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex)
				{
					if (m_Bones.find(node->mChildren[nodeIndex]->mName.C_Str()) != m_Bones.end())
						TraverseBone(node->mChildren[nodeIndex], skeleton, boneIndex);
				}
			}

			std::set<std::string> m_Bones;
			const aiScene* m_Scene = nullptr;
		};

		template<typename T>
		struct KeyFrame
		{
			float FrameTime = 0.0f;
			T Value{};
			KeyFrame(float frameTime, const T& value) : FrameTime(frameTime), Value(value) {}
		};

		struct Channel
		{
			std::vector<KeyFrame<glm::vec3>> Translations;
			std::vector<KeyFrame<glm::quat>> Rotations;
			std::vector<KeyFrame<glm::vec3>> Scales;
		};

		std::vector<Channel> ImportChannels(aiAnimation* anim, const Skeleton& skeleton)
		{
			std::vector<Channel> channels;
			std::unordered_map<std::string_view, uint32_t> boneIndices;
			for (uint32_t i = 0; i < skeleton.GetNumBones(); ++i)
				boneIndices.emplace(skeleton.GetBoneName(i), i + 1);

			std::map<uint32_t, aiNodeAnim*> validChannels;
			for (uint32_t channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex)
			{
				aiNodeAnim* nodeAnim = anim->mChannels[channelIndex];
				auto it = boneIndices.find(nodeAnim->mNodeName.C_Str());
				if (it != boneIndices.end())
					validChannels.emplace(it->second, nodeAnim);
			}

			channels.resize(skeleton.GetNumBones() + 1);

			double firstFrameDelta = DBL_MAX;
			double animationDuration = anim->mDuration;
			for (uint32_t boneIndex = 1; boneIndex < channels.size(); ++boneIndex)
			{
				if (auto validChannel = validChannels.find(boneIndex); validChannel != validChannels.end())
				{
					aiNodeAnim* nodeAnim = validChannel->second;
					if (nodeAnim->mNumPositionKeys > 0)
						firstFrameDelta = std::min(firstFrameDelta, nodeAnim->mPositionKeys[0].mTime);
					if (nodeAnim->mNumRotationKeys > 0)
						firstFrameDelta = std::min(firstFrameDelta, nodeAnim->mRotationKeys[0].mTime);
					if (nodeAnim->mNumScalingKeys > 0)
						firstFrameDelta = std::min(firstFrameDelta, nodeAnim->mScalingKeys[0].mTime);
				}
			}

			if (firstFrameDelta != DBL_MAX)
				anim->mDuration -= firstFrameDelta;
			if (anim->mDuration <= 0.0)
				anim->mDuration = 1.0;

			for (uint32_t boneIndex = 1; boneIndex < channels.size(); ++boneIndex)
			{
				Channel& channel = channels[boneIndex];
				if (auto validChannel = validChannels.find(boneIndex); validChannel != validChannels.end())
				{
					aiNodeAnim* nodeAnim = validChannel->second;
					for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumPositionKeys; ++keyIndex)
					{
						const aiVectorKey& key = nodeAnim->mPositionKeys[keyIndex];
						const float frameTime = glm::clamp(static_cast<float>((key.mTime - firstFrameDelta) / anim->mDuration), 0.0f, 1.0f);
						if (keyIndex == 0 && frameTime > 0.0f)
							channel.Translations.emplace_back(0.0f, glm::vec3((float)key.mValue.x, (float)key.mValue.y, (float)key.mValue.z));
						channel.Translations.emplace_back(frameTime, glm::vec3((float)key.mValue.x, (float)key.mValue.y, (float)key.mValue.z));
					}
					if (channel.Translations.empty())
						channel.Translations = { {0.0f, glm::vec3(0.0f)}, {1.0f, glm::vec3(0.0f)} };
					else if (channel.Translations.back().FrameTime < 1.0f)
						channel.Translations.emplace_back(1.0f, channel.Translations.back().Value);

					for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumRotationKeys; ++keyIndex)
					{
						const aiQuatKey& key = nodeAnim->mRotationKeys[keyIndex];
						const float frameTime = glm::clamp(static_cast<float>((key.mTime - firstFrameDelta) / anim->mDuration), 0.0f, 1.0f);
						const glm::quat rotation((float)key.mValue.w, (float)key.mValue.x, (float)key.mValue.y, (float)key.mValue.z);
						if (keyIndex == 0 && frameTime > 0.0f)
							channel.Rotations.emplace_back(0.0f, rotation);
						channel.Rotations.emplace_back(frameTime, rotation);
					}
					if (channel.Rotations.empty())
						channel.Rotations = { {0.0f, glm::quat(1,0,0,0)}, {1.0f, glm::quat(1,0,0,0)} };
					else if (channel.Rotations.back().FrameTime < 1.0f)
						channel.Rotations.emplace_back(1.0f, channel.Rotations.back().Value);

					for (uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumScalingKeys; ++keyIndex)
					{
						const aiVectorKey& key = nodeAnim->mScalingKeys[keyIndex];
						const float frameTime = glm::clamp(static_cast<float>((key.mTime - firstFrameDelta) / anim->mDuration), 0.0f, 1.0f);
						if (keyIndex == 0 && frameTime > 0.0f)
							channel.Scales.emplace_back(0.0f, glm::vec3((float)key.mValue.x, (float)key.mValue.y, (float)key.mValue.z));
						channel.Scales.emplace_back(frameTime, glm::vec3((float)key.mValue.x, (float)key.mValue.y, (float)key.mValue.z));
					}
					if (channel.Scales.empty())
						channel.Scales = { {0.0f, glm::vec3(1.0f)}, {1.0f, glm::vec3(1.0f)} };
					else if (channel.Scales.back().FrameTime < 1.0f)
						channel.Scales.emplace_back(1.0f, channel.Scales.back().Value);
				}
				else
				{
					const auto translation = skeleton.GetBoneTranslations().at(boneIndex - 1);
					const auto rotation = skeleton.GetBoneRotations().at(boneIndex - 1);
					const auto scale = skeleton.GetBoneScales().at(boneIndex - 1);
					channel.Translations = { {0.0f, translation}, {1.0f, translation} };
					channel.Rotations = { {0.0f, rotation}, {1.0f, rotation} };
					channel.Scales = { {0.0f, scale}, {1.0f, scale} };
				}
			}

			return channels;
		}

	}

	namespace AssimpAnimationImporter {

		Scope<Skeleton> ImportSkeleton(const aiScene* scene)
		{
			if (!scene)
				return nullptr;
			BoneHierarchy hierarchy(scene);
			return hierarchy.CreateSkeleton();
		}

		std::vector<std::string> GetAnimationNames(const aiScene* scene)
		{
			std::vector<std::string> animationNames;
			if (!scene)
				return animationNames;

			animationNames.reserve(scene->mNumAnimations);
			for (size_t i = 0; i < scene->mNumAnimations; ++i)
				animationNames.emplace_back(SanitiseAnimationName(scene->mAnimations[i]->mName.C_Str()));
			return animationNames;
		}

		uint32_t GetAnimationIndex(const aiScene* scene, std::string_view animationName)
		{
			if (!scene)
				return ~0u;

			for (size_t i = 0; i < scene->mNumAnimations; ++i)
			{
				if (SanitiseAnimationName(scene->mAnimations[i]->mName.C_Str()) == animationName)
					return static_cast<uint32_t>(i);
			}
			return ~0u;
		}

		Scope<AnimationClip> ImportAnimation(const aiScene* scene, uint32_t animationIndex, const Skeleton& skeleton)
		{
			if (!scene || animationIndex >= scene->mNumAnimations)
				return nullptr;

			aiAnimation* animation = scene->mAnimations[animationIndex];
			const std::vector<Channel> channels = ImportChannels(animation, skeleton);
			const float durationSeconds = animation->mTicksPerSecond != 0.0
				? static_cast<float>(animation->mDuration / animation->mTicksPerSecond)
				: static_cast<float>(animation->mDuration);

			std::vector<AnimationClip::BoneChannel> boneChannels;
			boneChannels.resize(channels.size());
			for (size_t i = 0; i < channels.size(); ++i)
			{
				for (const auto& key : channels[i].Translations)
					boneChannels[i].Translations.push_back({ key.FrameTime, key.Value });
				for (const auto& key : channels[i].Rotations)
					boneChannels[i].Rotations.push_back({ key.FrameTime, key.Value });
				for (const auto& key : channels[i].Scales)
					boneChannels[i].Scales.push_back({ key.FrameTime, key.Value });
			}

			return CreateScope<AnimationClip>(SanitiseAnimationName(animation->mName.C_Str()), durationSeconds, std::move(boneChannels));
		}

	}

}
