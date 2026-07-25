#pragma once

#include <string_view>

namespace Hazel {

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Prefab,
		Texture,
		MeshSource,
		StaticMesh,
		Material,
		Font,
		Shader,
		Skeleton,
		Animation,
		AnimationController,
	};

	inline std::string_view AssetTypeToString(AssetType type)
	{
		switch (type)
		{
		case AssetType::Scene:   return "Scene";
		case AssetType::Prefab:  return "Prefab";
		case AssetType::Texture:    return "Texture";
		case AssetType::MeshSource: return "MeshSource";
		case AssetType::StaticMesh: return "StaticMesh";
		case AssetType::Material: return "Material";
		case AssetType::Font:    return "Font";
		case AssetType::Shader:  return "Shader";
		case AssetType::Skeleton: return "Skeleton";
		case AssetType::Animation: return "Animation";
		case AssetType::AnimationController: return "AnimationController";
		default:                 return "None";
		}
	}

	inline AssetType AssetTypeFromString(std::string_view type)
	{
		if (type == "Scene")   return AssetType::Scene;
		if (type == "Prefab")  return AssetType::Prefab;
		if (type == "Texture")    return AssetType::Texture;
		if (type == "MeshSource") return AssetType::MeshSource;
		if (type == "StaticMesh") return AssetType::StaticMesh;
		if (type == "Material") return AssetType::Material;
		if (type == "Font")    return AssetType::Font;
		if (type == "Shader")  return AssetType::Shader;
		if (type == "Skeleton") return AssetType::Skeleton;
		if (type == "Animation") return AssetType::Animation;
		if (type == "AnimationController") return AssetType::AnimationController;
		return AssetType::None;
	}

}
