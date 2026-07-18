#pragma once

#include <entt/entt.hpp>
#include "Hazel/Core/Timestep.h"
#include <string>

namespace Hazel {

	class Entity; // 前向声明

	// Scene = 游戏世界：EnTT registry 里存 Entity + Components。
	// 与 Layer 的关系：Scene 不知道 LayerStack；由某个 Layer（或未来的 Editor）在 OnUpdate 里调用 OnUpdate。
	// 一个 Application 可有多层 Layer；通常「玩法世界」由其中一个 Layer 持有一个（或多个）Scene。
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		void OnUpdate(Timestep ts); // 脚本 + 找主相机 + Renderer2D 画所有 Sprite
		void OnViewportResize(uint32_t width, uint32_t height);
		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry; // entity → components 的容器
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
		
	};


}