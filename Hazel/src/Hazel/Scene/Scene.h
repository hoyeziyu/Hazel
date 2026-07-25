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
	class SceneRenderer;
	struct SceneEnvironmentData;

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
		void OnRenderEditor(SceneRenderer& renderer, const EditorCamera& camera, bool showGrid);
		void OnRenderRuntime(SceneRenderer& renderer, bool showGrid);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void CopyTo(const Ref<Scene>& target);

		void OnViewportResize(uint32_t width, uint32_t height);
		Entity GetPrimaryCameraEntity();

	private:
		void RenderScene();
		void RenderSprites();
		void RenderMeshes(SceneRenderer& renderer);
		void RenderScene3D(SceneRenderer& renderer, const glm::mat4& viewProjection, const glm::vec3& cameraPosition, bool showGrid);
		SceneEnvironmentData BuildSceneEnvironment();
		Entity CreatePrefabEntity(Entity entity, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale);

		template<typename T>
		void CopyComponentIfExists(Entity dst, Entity src);

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		uint64_t m_PhysicsWorldHandle = 0;
		bool m_Physics2DActive = false;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
		friend class Prefab;
		friend class PrefabSerializer;
		friend class Physics2DScene;
	};

}
