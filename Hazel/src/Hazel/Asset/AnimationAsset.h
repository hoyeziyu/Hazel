#pragma once

#include "Hazel/Asset/Asset.h"

namespace Hazel {

	class AnimationAsset : public Asset
	{
	public:
		AnimationAsset() = default;
		AnimationAsset(AssetHandle animationSource, AssetHandle skeletonSource, uint32_t animationIndex, std::string animationName = {});

		AssetType GetAssetType() const override { return AssetType::Animation; }

		AssetHandle GetAnimationSource() const { return m_AnimationSource; }
		AssetHandle GetSkeletonSource() const { return m_SkeletonSource; }
		uint32_t GetAnimationIndex() const { return m_AnimationIndex; }
		const std::string& GetAnimationName() const { return m_AnimationName; }

		void SetAnimationSource(AssetHandle handle) { m_AnimationSource = handle; }
		void SetSkeletonSource(AssetHandle handle) { m_SkeletonSource = handle; }
		void SetAnimationIndex(uint32_t index) { m_AnimationIndex = index; }
		void SetAnimationName(std::string name) { m_AnimationName = std::move(name); }

	private:
		AssetHandle m_AnimationSource = 0;
		AssetHandle m_SkeletonSource = 0;
		uint32_t m_AnimationIndex = 0;
		std::string m_AnimationName;
	};

}
