#pragma once

#include "AssetMetadata.h"

#include <unordered_map>

namespace Hazel {

	class AssetRegistry
	{
	public:
		bool Contains(AssetHandle handle) const;
		const AssetMetadata& Get(AssetHandle handle) const;
		void Set(const AssetMetadata& metadata);
		void Remove(AssetHandle handle);

		size_t Count() const { return m_Assets.size(); }

		const std::unordered_map<AssetHandle, AssetMetadata>& GetAssets() const { return m_Assets; }

	private:
		std::unordered_map<AssetHandle, AssetMetadata> m_Assets;
	};

}
