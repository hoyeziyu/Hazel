#pragma once

#include "Hazel/Animation/AnimationClip.h"
#include "Hazel/Animation/Skeleton.h"

struct aiScene;

namespace Hazel {

	namespace AssimpAnimationImporter {

		Scope<Skeleton> ImportSkeleton(const aiScene* scene);
		std::vector<std::string> GetAnimationNames(const aiScene* scene);
		uint32_t GetAnimationIndex(const aiScene* scene, std::string_view animationName);
		Scope<AnimationClip> ImportAnimation(const aiScene* scene, uint32_t animationIndex, const Skeleton& skeleton);

	}

}
