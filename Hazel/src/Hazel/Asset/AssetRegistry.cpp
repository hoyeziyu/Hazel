#include "hzpch.h"
#include "AssetRegistry.h"

namespace Hazel {

	static AssetMetadata s_NullMetadata;

	bool AssetRegistry::Contains(AssetHandle handle) const
	{
		return m_Assets.find(handle) != m_Assets.end();
	}

	const AssetMetadata& AssetRegistry::Get(AssetHandle handle) const
	{
		auto it = m_Assets.find(handle);
		if (it != m_Assets.end())
			return it->second;
		return s_NullMetadata;
	}

	void AssetRegistry::Set(const AssetMetadata& metadata)
	{
		m_Assets[metadata.Handle] = metadata;
	}

	void AssetRegistry::Remove(AssetHandle handle)
	{
		m_Assets.erase(handle);
	}

}
