#include "hzpch.h"
#include "Scene.h"
#include "Components.h"
#include <glm/glm.hpp>
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/SceneRenderer.h"
#include "Hazel/Editor/EditorCamera.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/TextureAsset.h"
#include "Hazel/Asset/MeshSource.h"
#include "Hazel/Asset/StaticMesh.h"
#include "Hazel/Asset/MaterialAsset.h"
#include "Hazel/Scene/Prefab.h"
#include "Entity.h"

#include <glm/gtc/matrix_transform.hpp>

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
		return CreateEntityWithID(UUID(), name);
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::GetEntityWithUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			if (view.get<IDComponent>(entity).ID == uuid)
				return Entity{ entity, this };
		}
		return {};
	}

	Ref<Scene> Scene::CreateEmpty()
	{
		return CreateRef<Scene>();
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::RenderSprites()
	{
		auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (auto entity : view)
		{
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

			if (sprite.Texture && AssetManager::IsAssetValid(sprite.Texture))
			{
				if (auto textureAsset = AssetManager::GetAsset<TextureAsset>(sprite.Texture))
				{
					Renderer2D::DrawQuad(transform.GetTransform(), textureAsset->Texture, 1.0f, sprite.Color);
					continue;
				}
			}

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

	void Scene::RenderMeshes(SceneRenderer& renderer)
	{
		auto staticMeshView = m_Registry.view<TransformComponent, StaticMeshComponent>();
		for (auto entity : staticMeshView)
		{
			auto [transform, mesh] = staticMeshView.get<TransformComponent, StaticMeshComponent>(entity);
			if (!mesh.Visible)
				continue;

			Ref<VertexArray> vertexArray = renderer.GetDefaultCubeMesh();
			if (mesh.StaticMesh && AssetManager::IsAssetHandleValid(mesh.StaticMesh))
			{
				if (auto staticMesh = AssetManager::GetAsset<StaticMesh>(mesh.StaticMesh))
				{
					if (auto meshSource = AssetManager::GetAsset<MeshSource>(staticMesh->GetMeshSource()))
					{
						if (meshSource->GetVertexArray())
							vertexArray = meshSource->GetVertexArray();
					}
				}
			}

			MeshMaterialData materialData;
			materialData.ColorTint = mesh.Color;
			materialData.AlbedoColor = glm::vec3(1.0f);

			if (mesh.Material && AssetManager::IsAssetHandleValid(mesh.Material))
			{
				if (auto materialAsset = AssetManager::GetAsset<MaterialAsset>(mesh.Material))
				{
					materialData.AlbedoColor = materialAsset->AlbedoColor;
					materialData.AlbedoTexture = materialAsset->GetAlbedoTexture();
				}
			}

			renderer.SubmitMesh(vertexArray, transform.GetTransform(), materialData);
		}

		auto legacyView = m_Registry.view<TransformComponent, MeshRendererComponent>();
		for (auto entity : legacyView)
		{
			auto [transform, mesh] = legacyView.get<TransformComponent, MeshRendererComponent>(entity);
			if (!mesh.Visible)
				continue;

			renderer.SubmitMesh(transform.GetTransform(), mesh.Color);
		}
	}

	void Scene::RenderScene3D(SceneRenderer& renderer, const glm::mat4& viewProjection, bool showGrid)
	{
		renderer.SetGridEnabled(showGrid);
		renderer.BeginScene(viewProjection);
		RenderMeshes(renderer);
		renderer.RenderGrid();
		renderer.EndScene();
	}

	void Scene::OnRenderEditor(SceneRenderer& renderer, const EditorCamera& camera, bool showGrid)
	{
		RenderScene3D(renderer, camera.GetViewProjection(), showGrid);

		renderer.Prepare2DOverlay();
		Renderer2D::BeginScene(camera.GetViewProjection());
		RenderSprites();
		Renderer2D::EndScene();
		renderer.RestoreAfter2D();
	}

	void Scene::OnRenderRuntime(SceneRenderer& renderer, bool showGrid)
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

		if (!mainCamera)
			return;

		glm::mat4 viewProjection = mainCamera->GetProjection() * glm::inverse(cameraTransform);
		RenderScene3D(renderer, viewProjection, showGrid);

		renderer.Prepare2DOverlay();
		Renderer2D::BeginScene(*mainCamera, cameraTransform);
		RenderSprites();
		Renderer2D::EndScene();
		renderer.RestoreAfter2D();
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

		CopyComponent<IDComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshRendererComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<StaticMeshComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<PrefabComponent>(target->m_Registry, m_Registry, enttMap);
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
	void Scene::CopyComponentIfExists(Entity dst, Entity src)
	{
		if (!src.HasComponent<T>())
			return;

		if (dst.HasComponent<T>())
			dst.GetComponent<T>() = src.GetComponent<T>();
		else
			dst.AddComponent<T>(src.GetComponent<T>());
	}

	Entity Scene::CreatePrefabEntity(Entity entity, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
	{
		HZ_CORE_ASSERT(entity.HasComponent<PrefabComponent>(), "Entity missing PrefabComponent");

		Entity newEntity = CreateEntity();
		CopyComponentIfExists<TagComponent>(newEntity, entity);
		CopyComponentIfExists<PrefabComponent>(newEntity, entity);
		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
		CopyComponentIfExists<StaticMeshComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);

		if (translation)
			newEntity.GetComponent<TransformComponent>().Translation = *translation;
		if (rotation)
			newEntity.GetComponent<TransformComponent>().Rotation = *rotation;
		if (scale)
			newEntity.GetComponent<TransformComponent>().Scale = *scale;

		return newEntity;
	}

	Entity Scene::Instantiate(const Ref<Prefab>& prefab, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
	{
		if (!prefab || !prefab->m_Entity)
			return {};

		return CreatePrefabEntity(prefab->m_Entity, translation, rotation, scale);
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		if (entity.HasComponent<PrefabComponent>())
		{
			auto prefabID = entity.GetComponent<PrefabComponent>().PrefabID;
			if (!AssetManager::IsAssetHandleValid(prefabID))
				return {};

			auto prefab = AssetManager::GetAsset<Prefab>(prefabID);
			if (!prefab)
				return {};

			const auto& transform = entity.GetComponent<TransformComponent>();
			glm::vec3 rotation = transform.Rotation;
			return Instantiate(prefab, &transform.Translation, &rotation, &transform.Scale);
		}

		Entity newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
		CopyComponentIfExists<StaticMeshComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		return newEntity;
	}

	std::unordered_set<AssetHandle> Scene::GetAssetList() const
	{
		std::unordered_set<AssetHandle> result;

		{
			auto view = m_Registry.view<SpriteRendererComponent>();
			for (auto entity : view)
			{
				const auto& sprite = view.get<SpriteRendererComponent>(entity);
				if (sprite.Texture && (uint64_t)sprite.Texture != 0)
					result.insert(sprite.Texture);
			}
		}

		{
			auto view = m_Registry.view<StaticMeshComponent>();
			for (auto entity : view)
			{
				const auto& mesh = view.get<StaticMeshComponent>(entity);
				if (mesh.StaticMesh && (uint64_t)mesh.StaticMesh != 0)
					result.insert(mesh.StaticMesh);
				if (mesh.Material && (uint64_t)mesh.Material != 0)
					result.insert(mesh.Material);
			}
		}

		{
			auto view = m_Registry.view<PrefabComponent>();
			for (auto entity : view)
			{
				const auto& prefab = view.get<PrefabComponent>(entity);
				if (prefab.PrefabID && (uint64_t)prefab.PrefabID != 0)
					result.insert(prefab.PrefabID);
			}
		}

		return result;
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}
	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity, IDComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<PrefabComponent>(Entity, PrefabComponent&)
	{
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
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity, MeshRendererComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<StaticMeshComponent>(Entity, StaticMeshComponent&)
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