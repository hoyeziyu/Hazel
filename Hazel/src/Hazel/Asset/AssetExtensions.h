#pragma once

#include "AssetTypes.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Hazel {

	inline AssetType GetAssetTypeFromExtension(const std::string& extension)
	{
		static const std::unordered_map<std::string, AssetType> s_Map = {
			{ ".hazel", AssetType::Scene },
			{ ".hprefab", AssetType::Prefab },
			{ ".png",   AssetType::Texture },
			{ ".jpg",   AssetType::Texture },
			{ ".jpeg",  AssetType::Texture },
			{ ".ttf",   AssetType::Font },
			{ ".glsl",  AssetType::Shader },
		};

		std::string ext = extension;
		for (char& c : ext)
			c = (char)std::tolower((unsigned char)c);

		auto it = s_Map.find(ext);
		if (it != s_Map.end())
			return it->second;
		return AssetType::None;
	}

	inline AssetType GetAssetTypeFromPath(const std::filesystem::path& path)
	{
		return GetAssetTypeFromExtension(path.extension().string());
	}

}
