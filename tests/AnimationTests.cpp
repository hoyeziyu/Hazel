#include <gtest/gtest.h>

#include "Hazel/Core/Core.h"
#include "Hazel/Animation/AnimationClip.h"
#include "Hazel/Animation/Skeleton.h"
#include "Hazel/Asset/AnimationControllerAsset.h"

namespace {

	Hazel::AnimationClip MakeTestClip()
	{
		Hazel::Skeleton skeleton(1);
		skeleton.AddBone("Root", Hazel::Skeleton::NullIndex, glm::mat4(1.0f));

		std::vector<Hazel::AnimationClip::BoneChannel> channels(2);
		channels[1].Translations = { {0.0f, glm::vec3(0.0f)}, {1.0f, glm::vec3(0.0f, 1.0f, 0.0f)} };
		channels[1].Rotations = { {0.0f, glm::quat(1,0,0,0)}, {1.0f, glm::quat(1,0,0,0)} };
		channels[1].Scales = { {0.0f, glm::vec3(1.0f)}, {1.0f, glm::vec3(1.0f)} };

		return Hazel::AnimationClip("Test", 1.0f, std::move(channels));
	}

}

TEST(AnimationTest, ClipSamplesBoneTranslation)
{
	Hazel::Skeleton skeleton(1);
	skeleton.AddBone("Root", Hazel::Skeleton::NullIndex, glm::mat4(1.0f));

	Hazel::AnimationClip clip = MakeTestClip();
	Hazel::AnimationPose pose = clip.Sample(0.5f, skeleton);

	ASSERT_EQ(pose.BonePoses.size(), 1u);
	EXPECT_NEAR(pose.BonePoses[0].Translation.y, 0.5f, 0.01f);
}

TEST(AnimationTest, ControllerAssetStoresStates)
{
	auto controller = Hazel::CreateRef<Hazel::AnimationControllerAsset>();
	controller->SetSkeletonAsset(123);

	Hazel::AnimationControllerState state;
	state.AnimationIndex = 1;
	state.Loop = true;
	controller->GetStates().push_back(state);
	controller->GetStates().push_back(state);

	EXPECT_EQ(controller->GetStates().size(), 2u);
	EXPECT_EQ((uint64_t)controller->GetSkeletonAsset(), 123ull);
}

TEST(AnimationTest, SkeletonBoneHierarchy)
{
	Hazel::Skeleton skeleton;
	const uint32_t root = skeleton.AddBone("Root", Hazel::Skeleton::NullIndex, glm::mat4(1.0f));
	const uint32_t child = skeleton.AddBone("Child", root, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	EXPECT_EQ(skeleton.GetNumBones(), 2u);
	EXPECT_EQ(skeleton.GetParentBoneIndex(child), root);
	EXPECT_EQ(skeleton.GetBoneIndex("Child"), child);
}
