#pragma once

#include <entt/entt.hpp>
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/Core.h"
#include <string>

namespace Hazel {

	class Entity;
	class EditorCamera;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

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

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};

}
