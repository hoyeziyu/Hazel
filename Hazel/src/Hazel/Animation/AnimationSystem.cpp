#include "hzpch.h"
#include "AnimationSystem.h"

#include "Hazel/Asset/AnimationAsset.h"
#include "Hazel/Asset/AnimationControllerAsset.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/MeshSource.h"
#include "Hazel/Asset/SkeletonAsset.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Hazel {

	namespace AnimationSystem {

		static AssetHandle ResolveMeshSourceFromSkeleton(AssetHandle skeletonAssetHandle)
		{
			if (!skeletonAssetHandle || !AssetManager::IsAssetHandleValid(skeletonAssetHandle))
				return 0;

			if (auto skeletonAsset = AssetManager::GetAsset<SkeletonAsset>(skeletonAssetHandle))
				return skeletonAsset->GetMeshSource();

			return 0;
		}

		const Skeleton* ResolveSkeleton(const AnimationComponent& component)
		{
			if (!component.AnimationController || !AssetManager::IsAssetHandleValid(component.AnimationController))
				return nullptr;

			auto controller = AssetManager::GetAsset<AnimationControllerAsset>(component.AnimationController);
			if (!controller)
				return nullptr;

			const AssetHandle meshSourceHandle = ResolveMeshSourceFromSkeleton(controller->GetSkeletonAsset());
			if (!meshSourceHandle)
				return nullptr;

			auto meshSource = AssetManager::GetAsset<MeshSource>(meshSourceHandle);
			if (!meshSource || !meshSource->HasSkeleton())
				return nullptr;

			return meshSource->GetSkeleton().get();
		}

		const AnimationClip* ResolveClipForState(const AnimationComponent& component, uint32_t stateIndex)
		{
			if (!component.AnimationController || !AssetManager::IsAssetHandleValid(component.AnimationController))
				return nullptr;

			auto controller = AssetManager::GetAsset<AnimationControllerAsset>(component.AnimationController);
			if (!controller || stateIndex >= controller->GetStates().size())
				return nullptr;

			const auto& state = controller->GetStates()[stateIndex];
			const AssetHandle meshSourceHandle = ResolveMeshSourceFromSkeleton(controller->GetSkeletonAsset());
			if (!meshSourceHandle)
				return nullptr;

			auto meshSource = AssetManager::GetAsset<MeshSource>(meshSourceHandle);
			if (!meshSource)
				return nullptr;

			if (state.AnimationAsset && AssetManager::IsAssetHandleValid(state.AnimationAsset))
			{
				if (auto animationAsset = AssetManager::GetAsset<AnimationAsset>(state.AnimationAsset))
					return meshSource->GetAnimation(animationAsset->GetAnimationIndex());
			}

			return meshSource->GetAnimation(state.AnimationIndex);
		}

		void ApplyPoseToBoneEntities(Scene& scene, const AnimationPose& pose, const std::vector<UUID>& boneEntities)
		{
			for (size_t boneIndex = 0; boneIndex < boneEntities.size() && boneIndex < pose.BonePoses.size(); ++boneIndex)
			{
				Entity boneEntity = scene.GetEntityWithUUID(boneEntities[boneIndex]);
				if (!boneEntity || !boneEntity.HasComponent<TransformComponent>())
					continue;

				auto& transform = boneEntity.GetComponent<TransformComponent>();
				const BonePose& bonePose = pose.BonePoses[boneIndex];
				transform.Translation = bonePose.Translation;
				transform.Rotation = glm::eulerAngles(bonePose.Rotation);
				transform.Scale = bonePose.Scale;
			}
		}

		std::vector<glm::mat4> ComputeSkinnedBoneMatrices(Scene& scene, const MeshSource& meshSource, const std::vector<UUID>& boneEntities)
		{
			std::vector<glm::mat4> result(meshSource.GetBoneInfo().size(), glm::mat4(1.0f));
			if (!meshSource.HasSkeleton())
				return result;

			const Skeleton& skeleton = *meshSource.GetSkeleton();
			std::vector<glm::mat4> modelSpaceTransforms(skeleton.GetNumBones(), glm::mat4(1.0f));

			for (uint32_t boneIndex = 0; boneIndex < skeleton.GetNumBones(); ++boneIndex)
			{
				glm::mat4 localTransform = glm::mat4(1.0f);
				if (boneIndex < boneEntities.size())
				{
					Entity boneEntity = scene.GetEntityWithUUID(boneEntities[boneIndex]);
					if (boneEntity && boneEntity.HasComponent<TransformComponent>())
						localTransform = boneEntity.GetComponent<TransformComponent>().GetTransform();
				}
				else
				{
					const auto& t = skeleton.GetBoneTranslations()[boneIndex];
					const auto& r = skeleton.GetBoneRotations()[boneIndex];
					const auto& s = skeleton.GetBoneScales()[boneIndex];
					localTransform = glm::translate(glm::mat4(1.0f), t) * glm::toMat4(r) * glm::scale(glm::mat4(1.0f), s);
				}

				const uint32_t parentIndex = skeleton.GetParentBoneIndex(boneIndex);
				if (parentIndex == Skeleton::NullIndex)
					modelSpaceTransforms[boneIndex] = skeleton.GetTransform() * localTransform;
				else
					modelSpaceTransforms[boneIndex] = modelSpaceTransforms[parentIndex] * localTransform;
			}

			for (size_t i = 0; i < meshSource.GetBoneInfo().size(); ++i)
			{
				const BoneInfo& boneInfo = meshSource.GetBoneInfo()[i];
				if (boneInfo.BoneIndex < modelSpaceTransforms.size())
					result[i] = modelSpaceTransforms[boneInfo.BoneIndex] * boneInfo.InverseBindPose;
			}

			return result;
		}

		void Update(Scene& scene, Timestep ts)
		{
			auto view = scene.GetRegistry().view<AnimationComponent>();
			for (auto entityID : view)
			{
				auto& animation = view.get<AnimationComponent>(entityID);

				if (!animation.IsAnimationPlaying || !animation.EnableAnimation)
					continue;

				const AnimationClip* clip = ResolveClipForState(animation, animation.StateIndex);
				if (!clip)
					continue;

				const float speed = animation.PlaybackSpeed > 0.0f ? animation.PlaybackSpeed : 1.0f;
				const float duration = clip->GetDurationSeconds() > 0.0f ? clip->GetDurationSeconds() : 1.0f;
				animation.AnimationTime += (float)ts * speed / duration;

				const Skeleton* skeleton = ResolveSkeleton(animation);
				if (!skeleton)
					continue;

				const auto& states = AssetManager::GetAsset<AnimationControllerAsset>(animation.AnimationController)->GetStates();
				const bool loop = animation.StateIndex < states.size() ? states[animation.StateIndex].Loop : true;
				if (loop)
				{
					if (animation.AnimationTime > 1.0f)
						animation.AnimationTime -= std::floor(animation.AnimationTime);
				}
				else
				{
					animation.AnimationTime = glm::clamp(animation.AnimationTime, 0.0f, 1.0f);
					if (animation.AnimationTime >= 1.0f)
						animation.IsAnimationPlaying = false;
				}

				AnimationPose pose = clip->Sample(animation.AnimationTime, *skeleton);
				ApplyPoseToBoneEntities(scene, pose, animation.BoneEntities);
			}
		}

	}

}
