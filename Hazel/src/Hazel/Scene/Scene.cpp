#include "hzpch.h"
#include "Scene.h"
#include "Components.h"
#include <glm/glm.hpp>
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Editor/EditorCamera.h"
#include "Entity.h"

#include <unordered_map>

namespace Hazel {

#if EXAMPLE_ENTT
	static void OnTransformConstruct(entt::registry&, entt::entity)
	{
	}
#endif

	Scene::Scene()
	{
#if EXAMPLE_ENTT
		struct MeshComponent{
			bool Data;
			MeshComponent() = default;
		};

		entt::entity entity = m_Registry.create();
		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));
		m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
		if (m_Registry.any_of<TransformComponent>(entity))
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transform = view.get<TransformComponent>(entity);
		}
		auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto entity : group)
		{
			// todo: group.get fails when MeshComponent is empty (zero-size component)
			// EnTT does not support get() with zero-size components.
			auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);	
		}
#endif

	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::RenderSprites()
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
		}
	}

	void Scene::RenderScene()
	{
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);
			RenderSprites();
			Renderer2D::EndScene();
		}
	}

	void Scene::OnRenderEditor(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera.GetViewProjection());
		RenderSprites();
		Renderer2D::EndScene();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
				if (!nsc.Instance) {
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };

					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});
		}

		RenderScene();
	}

	void Scene::OnUpdateEditor(Timestep ts)
	{
		(void)ts;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		OnUpdateRuntime(ts);
	}

	void Scene::OnRuntimeStart()
	{
	}

	void Scene::OnRuntimeStop()
	{
		m_Registry.view<NativeScriptComponent>().each([](auto, auto& nsc) {
			if (nsc.Instance && nsc.DestroyScript)
			{
				nsc.DestroyScript(&nsc);
				nsc.Instance = nullptr;
			}
		});
	}

	namespace {

		template<typename T>
		static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<entt::entity, entt::entity>& enttMap)
		{
			auto view = src.view<T>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(srcEntity);
				auto& srcComponent = src.get<T>(srcEntity);
				dst.emplace_or_replace<T>(dstEntity, srcComponent);
			}
		}

	}

	void Scene::CopyTo(const Ref<Scene>& target)
	{
		target->m_ViewportWidth = m_ViewportWidth;
		target->m_ViewportHeight = m_ViewportHeight;

		std::unordered_map<entt::entity, entt::entity> enttMap;
		{
			auto view = m_Registry.view<TagComponent>();
			for (auto entity : view)
			{
				const auto& tag = m_Registry.get<TagComponent>(entity);
				Entity newEntity = target->CreateEntity(tag.Tag);
				enttMap[entity] = newEntity;
			}
		}

		CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<NativeScriptComponent>(target->m_Registry, m_Registry, enttMap);

		target->m_Registry.view<NativeScriptComponent>().each([](auto, auto& nsc) {
			nsc.Instance = nullptr;
		});
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		// Resize non-fixed-aspect cameras when the viewport changes
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}

	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity, TransformComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity, SpriteRendererComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity, TagComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity, NativeScriptComponent&)
	{
	}

}