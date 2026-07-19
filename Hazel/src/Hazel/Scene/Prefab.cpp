#include "hzpch.h"
#include "Prefab.h"

#include "Scene.h"
#include "Entity.h"
#include "Components.h"

#include "Hazel/Asset/AssetImporter.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Project/Project.h"

namespace Hazel {

	Entity Prefab::CreatePrefabFromEntity(Entity entity)
	{
		HZ_CORE_ASSERT((uint64_t)Handle != 0, "Prefab handle is invalid");

		Entity newEntity = m_Scene->CreateEntity();
		newEntity.AddComponent<PrefabComponent>(Handle, newEntity.GetUUID());

		if (entity.HasComponent<TagComponent>())
			newEntity.GetComponent<TagComponent>().Tag = entity.GetComponent<TagComponent>().Tag;

		if (entity.HasComponent<TransformComponent>())
			newEntity.GetComponent<TransformComponent>() = entity.GetComponent<TransformComponent>();

		if (entity.HasComponent<SpriteRendererComponent>())
			newEntity.AddComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>());

		if (entity.HasComponent<CameraComponent>())
			newEntity.AddComponent<CameraComponent>(entity.GetComponent<CameraComponent>());

		return newEntity;
	}

	Prefab::Prefab()
	{
		m_Scene = Scene::CreateEmpty();
	}

	Prefab::~Prefab()
	{
	}

	void Prefab::Create(Entity entity, bool serialize)
	{
		m_Scene = Scene::CreateEmpty();
		m_Entity = CreatePrefabFromEntity(entity);

		if (serialize && (uint64_t)Handle != 0)
			AssetImporter::Serialize(std::static_pointer_cast<Asset>(shared_from_this()));
	}

	std::unordered_set<AssetHandle> Prefab::GetAssetList(bool recursive)
	{
		std::unordered_set<AssetHandle> prefabAssetList = m_Scene->GetAssetList();
		if (!recursive)
			return prefabAssetList;

		for (AssetHandle handle : prefabAssetList)
		{
			if (!AssetManager::IsAssetHandleValid(handle))
				continue;

			if (AssetManager::GetAssetType(handle) == AssetType::Prefab)
			{
				if (auto prefab = AssetManager::GetAsset<Prefab>(handle))
				{
					auto childAssets = prefab->GetAssetList(true);
					prefabAssetList.insert(childAssets.begin(), childAssets.end());
				}
			}
		}

		return prefabAssetList;
	}

}
