#include "hzpch.h"
#include "AnimationAsset.h"

namespace Hazel {

	AnimationAsset::AnimationAsset(AssetHandle animationSource, AssetHandle skeletonSource, uint32_t animationIndex, std::string animationName)
		: m_AnimationSource(animationSource)
		, m_SkeletonSource(skeletonSource)
		, m_AnimationIndex(animationIndex)
		, m_AnimationName(std::move(animationName))
	{
	}

}
