#pragma once

#include "Hazel/Asset/Asset.h"

#include "Hazel/Scene/Entity.h"

#include <memory>

namespace Hazel {

	class Scene;
	class Entity;

	class Prefab : public Asset, public std::enable_shared_from_this<Prefab>
	{
	public:
		Prefab();
		~Prefab();

		void Create(Entity entity, bool serialize = true);

		AssetType GetAssetType() const override { return AssetType::Prefab; }

		std::unordered_set<AssetHandle> GetAssetList(bool recursive = true);

		bool LoadFromYAML(const std::string& yamlString);

		Ref<Scene>& GetScene() { return m_Scene; }
		const Ref<Scene>& GetScene() const { return m_Scene; }
		Entity GetRootEntity() const { return m_Entity; }

	private:
		Entity CreatePrefabFromEntity(Entity entity);

	private:
		Ref<Scene> m_Scene;
		Entity m_Entity;

		friend class Scene;
		friend class PrefabSerializer;
		friend class SceneSerializer;
	};

}
