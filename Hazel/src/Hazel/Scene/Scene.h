#pragma once

#include <entt/entt.hpp>
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/Core.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Asset/Asset.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_set>

namespace Hazel {

	class Entity;
	class EditorCamera;
	class Prefab;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithID(UUID uuid, const std::string& name = std::string());
		Entity GetEntityWithUUID(UUID uuid);
		void DestroyEntity(Entity entity);

		Entity DuplicateEntity(Entity entity);
		Entity Instantiate(const Ref<Prefab>& prefab, const glm::vec3* translation = nullptr, const glm::vec3* rotation = nullptr, const glm::vec3* scale = nullptr);

		std::unordered_set<AssetHandle> GetAssetList() const;

		static Ref<Scene> CreateEmpty();

		void OnUpdate(Timestep ts);
		void OnUpdateEditor(Timestep ts);
		void OnUpdateRuntime(Timestep ts);
		void OnRenderEditor(const EditorCamera& camera);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void CopyTo(const Ref<Scene>& target);

		void OnViewportResize(uint32_t width, uint32_t height);
		Entity GetPrimaryCameraEntity();

	private:
		void RenderScene();
		void RenderSprites();
		Entity CreatePrefabEntity(Entity entity, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale);

		template<typename T>
		void CopyComponentIfExists(Entity dst, Entity src);

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
		friend class Prefab;
		friend class PrefabSerializer;
	};

}
