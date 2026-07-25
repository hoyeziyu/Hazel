#pragma once

#include "Hazel/Animation/AnimationClip.h"
#include "Hazel/Asset/MeshSource.h"
#include "Hazel/Scene/Components.h"

#include <glm/glm.hpp>
#include <vector>

namespace Hazel {

	class Scene;
	struct Timestep;

	namespace AnimationSystem {

		const AnimationClip* ResolveClipForState(const AnimationComponent& component, uint32_t stateIndex);
		const Skeleton* ResolveSkeleton(const AnimationComponent& component);

		void ApplyPoseToBoneEntities(Scene& scene, const AnimationPose& pose, const std::vector<UUID>& boneEntities);
		std::vector<glm::mat4> ComputeSkinnedBoneMatrices(Scene& scene, const MeshSource& meshSource, const std::vector<UUID>& boneEntities);

		void Update(Scene& scene, Timestep ts);

	}

}
