#pragma once

#include "Hazel/Asset/Asset.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Hazel {

	struct AnimationControllerState
	{
		std::string Name;
		AssetHandle AnimationAsset = 0;
		uint32_t AnimationIndex = 0;
		bool Loop = true;
		glm::vec3 RootTranslationMask{ 0.0f };
		float RootRotationMask = 0.0f;
		glm::vec3 RootTranslationExtractMask{ 0.0f };
		float RootRotationExtractMask = 0.0f;
	};

	class AnimationControllerAsset : public Asset
	{
	public:
		AssetType GetAssetType() const override { return AssetType::AnimationController; }

		AssetHandle GetSkeletonAsset() const { return m_SkeletonAsset; }
		void SetSkeletonAsset(AssetHandle handle) { m_SkeletonAsset = handle; }

		const std::vector<AnimationControllerState>& GetStates() const { return m_States; }
		std::vector<AnimationControllerState>& GetStates() { return m_States; }

	private:
		AssetHandle m_SkeletonAsset = 0;
		std::vector<AnimationControllerState> m_States;
	};

}
