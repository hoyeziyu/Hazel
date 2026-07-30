#include "hzpch.h"
#include "Scene.h"
#include "Components.h"
#include <glm/glm.hpp>
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/SceneRenderer.h"
#include "Hazel/Renderer/SceneEnvironment.h"
#include "Hazel/Editor/EditorCamera.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/TextureAsset.h"
#include "Hazel/Asset/MeshSource.h"
#include "Hazel/Asset/StaticMesh.h"
#include "Hazel/Asset/MaterialAsset.h"
#include "Hazel/Scene/Prefab.h"
#include "Hazel/Physics2D/Physics2DScene.h"
#include "Hazel/Script/NativeScriptFactory.h"
#include "Hazel/Script/NativeScriptRegistry.h"
#include "Hazel/Script/ScriptEngine.h"
#include "Hazel/Core/Time.h"
#include "Hazel/Debug/RuntimeHUD.h"
#include "Hazel/Audio/AudioEngine.h"
#include "Hazel/Animation/AnimationSystem.h"
#include "Hazel/Project/Project.h"
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
		m_ScriptStorage.Clear();
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

	Entity Scene::TryGetEntityWithTag(const std::string& tag)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto& tagComponent = view.get<TagComponent>(entity);
			if (tagComponent.Tag == tag)
				return { entity, this };
		}
		return {};
	}

	std::vector<UUID> Scene::GetAllEntityUUIDs() const
	{
		std::vector<UUID> result;
		auto view = m_Registry.view<IDComponent>();
		result.reserve(view.size());
		for (auto entity : view)
			result.push_back(view.get<IDComponent>(entity).ID);
		return result;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.HasComponent<AudioComponent>())
		{
			auto& audio = entity.GetComponent<AudioComponent>();
			if (audio.RuntimeHandle != 0)
			{
				AudioEngine::Get().StopSound(audio.RuntimeHandle);
				audio.RuntimeHandle = 0;
			}
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			const auto& sc = entity.GetComponent<ScriptComponent>();
			if (sc.ScriptID && m_ScriptStorage.EntityStorage.contains(entity.GetUUID()))
				m_ScriptStorage.ShutdownEntityStorage(sc.ScriptID, entity.GetUUID());
		}

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

		auto skinnedView = m_Registry.view<TransformComponent, SkinnedMeshComponent>();
		for (auto entity : skinnedView)
		{
			auto [transform, mesh] = skinnedView.get<TransformComponent, SkinnedMeshComponent>(entity);
			if (!mesh.Visible || !mesh.StaticMesh || !AssetManager::IsAssetHandleValid(mesh.StaticMesh))
				continue;

			auto staticMesh = AssetManager::GetAsset<StaticMesh>(mesh.StaticMesh);
			if (!staticMesh)
				continue;

			auto meshSource = AssetManager::GetAsset<MeshSource>(staticMesh->GetMeshSource());
			if (!meshSource || !meshSource->IsRigged())
				continue;

			const Ref<VertexArray>& vertexArray = meshSource->GetSkinnedVertexArray();
			if (!vertexArray)
				continue;

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

			std::vector<glm::mat4> boneMatrices = AnimationSystem::ComputeSkinnedBoneMatrices(*this, *meshSource, mesh.BoneEntities);
			renderer.SubmitSkinnedMesh(vertexArray, transform.GetTransform(), materialData, boneMatrices);
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

	void Scene::RenderScene3D(SceneRenderer& renderer, const glm::mat4& viewProjection, const glm::vec3& cameraPosition, bool showGrid)
	{
		SceneEnvironmentData environment = BuildSceneEnvironment();
		environment.CameraPosition = cameraPosition;

		renderer.SetGridEnabled(showGrid);
		renderer.BeginScene(viewProjection, environment);
		RenderMeshes(renderer);
		renderer.RenderGrid();
		renderer.EndScene();
	}

	SceneEnvironmentData Scene::BuildSceneEnvironment()
	{
		SceneEnvironmentData environment;

		auto view = m_Registry.view<TransformComponent, DirectionalLightComponent>();
		for (auto entity : view)
		{
			auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
			environment.LightDirection = -glm::normalize(glm::mat3(transform.GetTransform()) * glm::vec3(1.0f));
			environment.LightRadiance = light.Radiance;
			environment.LightIntensity = light.Intensity;
			environment.HasDirectionalLight = true;
			break;
		}

		return environment;
	}

	void Scene::OnRenderEditor(SceneRenderer& renderer, const EditorCamera& camera, bool showGrid)
	{
		RenderScene3D(renderer, camera.GetViewProjection(), camera.GetPosition(), showGrid);

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
		glm::vec3 cameraPosition = glm::vec3(cameraTransform[3]);
		RenderScene3D(renderer, viewProjection, cameraPosition, showGrid);

		renderer.Prepare2DOverlay();
		Renderer2D::BeginScene(*mainCamera, cameraTransform);
		RenderSprites();
		Renderer2D::EndScene();
		renderer.RestoreAfter2D();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		ts = ts * Time::GetTimeScale();

		Physics2DScene::Step(*this, ts);
		AnimationSystem::Update(*this, ts);

		// Native scripts
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

		// C# scripts
		{
			const auto& scriptEngine = ScriptEngine::GetInstance();
			m_Registry.view<ScriptComponent>().each([&](auto, ScriptComponent& sc) {
				if (!scriptEngine.IsValidScript(sc.ScriptID) || !sc.Instance.IsValid())
					return;

				sc.Instance.Invoke<float>("OnUpdate", (float)ts);
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
		RuntimeHUD::Clear();
		Time::Reset();

		RegisterBuiltInNativeScripts();
		NativeScriptFactory::BindSceneScripts(*this);
		Physics2DScene::Init(*this);

		auto& audioEngine = AudioEngine::Get();
		auto project = Project::GetActive();

		m_Registry.view<AudioComponent>().each([&](auto, AudioComponent& ac) {
			if (!ac.PlayOnAwake)
				return;

			if (ac.SoundConfig)
			{
				ac.RuntimeHandle = audioEngine.PlaySoundConfig(ac.SoundConfig);
				return;
			}

			if (ac.FilePath.empty())
				return;

			std::filesystem::path path = ac.FilePath;
			if (project)
				path = project->GetAssetPath(ac.FilePath);

			ac.RuntimeHandle = audioEngine.PlaySound(path, ac.Volume, ac.Loop);
		});

		auto& scriptEngine = ScriptEngine::GetMutable();

		auto view = m_Registry.view<IDComponent, ScriptComponent>();
		for (auto entityID : view)
		{
			const auto& idComponent = view.get<IDComponent>(entityID);
			auto& scriptComponent = view.get<ScriptComponent>(entityID);

			if (!scriptEngine.IsValidScript(scriptComponent.ScriptID))
			{
				HZ_CORE_WARN("[Scripting] Entity has invalid script id {}", (uint64_t)scriptComponent.ScriptID);
				continue;
			}

			if (!m_ScriptStorage.EntityStorage.contains(idComponent.ID))
				m_ScriptStorage.InitializeEntityStorage(scriptComponent.ScriptID, idComponent.ID);

			scriptComponent.Instance = scriptEngine.Instantiate(idComponent.ID, m_ScriptStorage, (uint64_t)idComponent.ID);
		}

		for (auto entityID : view)
		{
			auto& scriptComponent = view.get<ScriptComponent>(entityID);
			if (!scriptEngine.IsValidScript(scriptComponent.ScriptID) || !scriptComponent.Instance.IsValid())
				continue;

			scriptComponent.Instance.Invoke("OnCreate");
		}
	}

	void Scene::OnRuntimeStop()
	{
		RuntimeHUD::Clear();
		Time::Reset();

		Physics2DScene::Shutdown(*this);

		m_Registry.view<AudioComponent>().each([](auto, AudioComponent& ac) {
			if (ac.RuntimeHandle != 0)
			{
				AudioEngine::Get().StopSound(ac.RuntimeHandle);
				ac.RuntimeHandle = 0;
			}
		});

		m_Registry.view<NativeScriptComponent>().each([](auto, auto& nsc) {
			if (nsc.Instance && nsc.DestroyScript)
			{
				nsc.DestroyScript(&nsc);
				nsc.Instance = nullptr;
			}
		});

		auto& scriptEngine = ScriptEngine::GetMutable();
		m_Registry.view<IDComponent, ScriptComponent>().each([&](auto entityID, IDComponent& idComponent, ScriptComponent& sc) {
			if (sc.Instance.IsValid())
			{
				sc.Instance.Invoke("OnDestroy");
				scriptEngine.DestroyInstance(idComponent.ID, m_ScriptStorage);
				sc.Instance = {};
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
		CopyComponent<DirectionalLightComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<RigidBody2DComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxCollider2DComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<AudioComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<AnimationComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SkinnedMeshComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<HierarchyComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<PrefabComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<NativeScriptComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<ScriptComponent>(target->m_Registry, m_Registry, enttMap);

		target->m_Registry.view<AudioComponent>().each([](auto, auto& ac) {
			ac.RuntimeHandle = 0;
		});

		target->m_Registry.view<NativeScriptComponent>().each([](auto, auto& nsc) {
			nsc.Instance = nullptr;
			nsc.InstantiateScript = nullptr;
			nsc.DestroyScript = nullptr;
		});

		target->m_Registry.view<ScriptComponent>().each([](auto, ScriptComponent& sc) {
			sc.Instance = {};
		});

		target->m_Registry.view<RigidBody2DComponent>().each([](auto, auto& rb) {
			rb.RuntimeBodyHandle = 0;
		});
		target->m_Registry.view<BoxCollider2DComponent>().each([](auto, auto& box) {
			box.RuntimeShapeHandle = 0;
		});

		m_ScriptStorage.CopyTo(target->m_ScriptStorage);
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
		CopyComponentIfExists<HierarchyComponent>(newEntity, entity);
		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
		CopyComponentIfExists<StaticMeshComponent>(newEntity, entity);
		CopyComponentIfExists<DirectionalLightComponent>(newEntity, entity);
		CopyComponentIfExists<RigidBody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<AudioComponent>(newEntity, entity);
		CopyComponentIfExists<AnimationComponent>(newEntity, entity);
		CopyComponentIfExists<SkinnedMeshComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);

		if (entity.HasComponent<ScriptComponent>())
		{
			const auto& srcScript = entity.GetComponent<ScriptComponent>();
			if (srcScript.ScriptID)
			{
				newEntity.AddComponent<ScriptComponent>().ScriptID = srcScript.ScriptID;
				m_ScriptStorage.InitializeEntityStorage(srcScript.ScriptID, newEntity.GetUUID());
				m_ScriptStorage.CopyEntityStorage(entity.GetUUID(), newEntity.GetUUID(), m_ScriptStorage);
			}
		}

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
		if (!prefab || !prefab->m_Scene || !prefab->m_Entity)
			return {};

		Ref<Scene> sourceScene = prefab->m_Scene;
		std::unordered_map<UUID, UUID> uuidRemap;
		std::vector<Entity> sourceEntities;
		sourceEntities.reserve(sourceScene->m_Registry.view<IDComponent>().size());

		sourceScene->m_Registry.view<IDComponent>().each([&](auto entityHandle, auto&)
			{
				sourceEntities.emplace_back(entityHandle, sourceScene.get());
			});

		if (sourceEntities.empty())
			return {};

		for (Entity sourceEntity : sourceEntities)
			uuidRemap[sourceEntity.GetUUID()] = UUID();

		auto remapUuid = [&uuidRemap](UUID uuid) -> UUID
			{
				if ((uint64_t)uuid == 0)
					return UUID(0);

				auto it = uuidRemap.find(uuid);
				return it != uuidRemap.end() ? it->second : UUID(0);
			};

		Entity rootEntity;
		for (Entity sourceEntity : sourceEntities)
		{
			const UUID newUuid = uuidRemap.at(sourceEntity.GetUUID());
			std::string name = "Entity";
			if (sourceEntity.HasComponent<TagComponent>())
				name = sourceEntity.GetComponent<TagComponent>().Tag;

			Entity newEntity = CreateEntityWithID(newUuid, name);

			CopyComponentIfExists<TagComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<TransformComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<SpriteRendererComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<MeshRendererComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<StaticMeshComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<DirectionalLightComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<RigidBody2DComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<BoxCollider2DComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<CameraComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<AudioComponent>(newEntity, sourceEntity);
			CopyComponentIfExists<NativeScriptComponent>(newEntity, sourceEntity);

			if (sourceEntity.HasComponent<PrefabComponent>())
			{
				const auto& srcPrefab = sourceEntity.GetComponent<PrefabComponent>();
				newEntity.AddComponent<PrefabComponent>(srcPrefab.PrefabID, remapUuid(srcPrefab.EntityID));
			}

			if (sourceEntity.HasComponent<HierarchyComponent>())
			{
				const auto& srcHierarchy = sourceEntity.GetComponent<HierarchyComponent>();
				auto& hierarchy = newEntity.AddComponent<HierarchyComponent>();
				hierarchy.Parent = remapUuid(srcHierarchy.Parent);
				hierarchy.Children.reserve(srcHierarchy.Children.size());
				for (UUID child : srcHierarchy.Children)
					hierarchy.Children.push_back(remapUuid(child));
			}

			if (sourceEntity.HasComponent<AnimationComponent>())
			{
				newEntity.AddComponent<AnimationComponent>(sourceEntity.GetComponent<AnimationComponent>());
				auto& anim = newEntity.GetComponent<AnimationComponent>();
				for (UUID& boneId : anim.BoneEntities)
					boneId = remapUuid(boneId);
			}

			if (sourceEntity.HasComponent<SkinnedMeshComponent>())
			{
				newEntity.AddComponent<SkinnedMeshComponent>(sourceEntity.GetComponent<SkinnedMeshComponent>());
				auto& mesh = newEntity.GetComponent<SkinnedMeshComponent>();
				for (UUID& boneId : mesh.BoneEntities)
					boneId = remapUuid(boneId);
			}

			if (sourceEntity.HasComponent<ScriptComponent>())
			{
				const auto& srcScript = sourceEntity.GetComponent<ScriptComponent>();
				if (srcScript.ScriptID)
				{
					newEntity.AddComponent<ScriptComponent>().ScriptID = srcScript.ScriptID;
					m_ScriptStorage.InitializeEntityStorage(srcScript.ScriptID, newEntity.GetUUID());
					sourceScene->GetScriptStorage().CopyEntityStorage(sourceEntity.GetUUID(), newEntity.GetUUID(), m_ScriptStorage);
				}
			}

			if (sourceEntity == prefab->m_Entity)
				rootEntity = newEntity;
		}

		if (!rootEntity)
			return {};

		if (translation)
			rootEntity.GetComponent<TransformComponent>().Translation = *translation;
		if (rotation)
			rootEntity.GetComponent<TransformComponent>().Rotation = *rotation;
		if (scale)
			rootEntity.GetComponent<TransformComponent>().Scale = *scale;

		return rootEntity;
	}

	void Scene::SetParent(Entity child, Entity parent)
	{
		if (!child)
			return;

		UUID oldParent = 0;
		if (child.HasComponent<HierarchyComponent>())
			oldParent = child.GetComponent<HierarchyComponent>().Parent;

		if ((uint64_t)oldParent != 0)
		{
			Entity oldParentEntity = GetEntityWithUUID(oldParent);
			if (oldParentEntity && oldParentEntity.HasComponent<HierarchyComponent>())
			{
				auto& siblings = oldParentEntity.GetComponent<HierarchyComponent>().Children;
				siblings.erase(std::remove(siblings.begin(), siblings.end(), child.GetUUID()), siblings.end());
			}
		}

		if (!child.HasComponent<HierarchyComponent>())
			child.AddComponent<HierarchyComponent>();

		auto& hierarchy = child.GetComponent<HierarchyComponent>();
		hierarchy.Parent = parent ? parent.GetUUID() : UUID(0);

		if (!parent)
			return;

		if (!parent.HasComponent<HierarchyComponent>())
			parent.AddComponent<HierarchyComponent>();

		auto& parentChildren = parent.GetComponent<HierarchyComponent>().Children;
		if (std::find(parentChildren.begin(), parentChildren.end(), child.GetUUID()) == parentChildren.end())
			parentChildren.push_back(child.GetUUID());
	}

	Entity Scene::InstantiateChild(const Ref<Prefab>& prefab, Entity parent, const glm::vec3* translation, const glm::vec3* rotation, const glm::vec3* scale)
	{
		Entity instance = Instantiate(prefab, translation, rotation, scale);
		if (instance && parent)
			SetParent(instance, parent);
		return instance;
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
		CopyComponentIfExists<DirectionalLightComponent>(newEntity, entity);
		CopyComponentIfExists<RigidBody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<AudioComponent>(newEntity, entity);
		CopyComponentIfExists<AnimationComponent>(newEntity, entity);
		CopyComponentIfExists<SkinnedMeshComponent>(newEntity, entity);
		CopyComponentIfExists<HierarchyComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);

		if (entity.HasComponent<ScriptComponent>())
		{
			const auto& srcScript = entity.GetComponent<ScriptComponent>();
			if (srcScript.ScriptID)
			{
				newEntity.AddComponent<ScriptComponent>().ScriptID = srcScript.ScriptID;
				m_ScriptStorage.InitializeEntityStorage(srcScript.ScriptID, newEntity.GetUUID());
				m_ScriptStorage.CopyEntityStorage(entity.GetUUID(), newEntity.GetUUID(), m_ScriptStorage);
			}
		}

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

		{
			auto view = m_Registry.view<AudioComponent>();
			for (auto entity : view)
			{
				const auto& audio = view.get<AudioComponent>(entity);
				if (audio.SoundConfig && (uint64_t)audio.SoundConfig != 0)
					result.insert(audio.SoundConfig);
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
	void Scene::OnComponentAdded<HierarchyComponent>(Entity, HierarchyComponent&)
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
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity, DirectionalLightComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<RigidBody2DComponent>(Entity, RigidBody2DComponent&)
	{
	}
	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity, BoxCollider2DComponent&)
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
	template<>
	void Scene::OnComponentAdded<AudioComponent>(Entity, AudioComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<AnimationComponent>(Entity, AnimationComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkinnedMeshComponent>(Entity, SkinnedMeshComponent&)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
		if (component.ScriptID && ScriptEngine::GetInstance().IsValidScript(component.ScriptID))
		{
			if (!m_ScriptStorage.EntityStorage.contains(entity.GetUUID()))
				m_ScriptStorage.InitializeEntityStorage(component.ScriptID, entity.GetUUID());
		}
	}

}