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
		Font,
		Shader,
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
		case AssetType::Font:    return "Font";
		case AssetType::Shader:  return "Shader";
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
		if (type == "Font")    return AssetType::Font;
		if (type == "Shader")  return AssetType::Shader;
		return AssetType::None;
	}

}
