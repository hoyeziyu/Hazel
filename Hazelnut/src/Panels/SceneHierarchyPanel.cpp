#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <filesystem>
#include <string>
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/AssetTypes.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Scene/Prefab.h"
#include <glm/gtc/type_ptr.hpp>
namespace Hazel {




	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		// ???new?????????????????????????????????????????????????????
		// ?????????????????????????????????????????????
		m_SelectionContext = {}; 
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		
		m_Context->m_Registry.view<entt::entity>().each([&](auto entityID)
			{
				Entity entity(entityID, m_Context.get());
				DrawEntityNode(entity);
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Context->CreateEntity("Empty Entity");

			if (m_SelectionContext)
			{
				ImGui::Separator();
				if (ImGui::MenuItem("Create Prefab from Selected..."))
					CreatePrefabFromEntity(m_SelectionContext);
			}

			ImGui::EndPopup();
		}

		ImGui::End();
		
		ImGui::Begin("Properties");
		if (m_SelectionContext) {
			DrawPrefabActions(m_SelectionContext);
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();

		//ImGui::ShowDemoWindow();	// demo??????????
	}

	void SceneHierarchyPanel::CreatePrefabFromEntity(Entity entity)
	{
		if (!Project::GetActive())
		{
			HZ_CORE_WARN("Open a project before creating prefabs.");
			return;
		}

		if (!entity || !entity.HasComponent<TagComponent>())
			return;

		const std::string& tag = entity.GetComponent<TagComponent>().Tag;
		const std::string prefabName = tag + ".hprefab";
		const auto relativePath = std::filesystem::path("prefabs") / prefabName;

		AssetHandle handle = AssetManager::CreateNewAsset(relativePath);
		if ((uint64_t)handle == 0)
		{
			HZ_CORE_WARN("Failed to create prefab asset at {}", relativePath.generic_string());
			return;
		}

		auto prefab = CreateRef<Prefab>();
		prefab->Handle = handle;
		AssetManager::SetLoadedAsset(handle, prefab);
		prefab->Create(entity, true);
		HZ_CORE_INFO("Created prefab {}", relativePath.generic_string());
	}

	void SceneHierarchyPanel::DrawPrefabActions(Entity entity)
	{
		if (!entity)
			return;

		const bool isPrefabInstance = entity.HasComponent<PrefabComponent>();

		if (ImGui::Button("Create Prefab"))
			CreatePrefabFromEntity(entity);

		ImGui::SameLine();

		ImGui::BeginDisabled(!isPrefabInstance);
		if (ImGui::Button("Update Prefab"))
		{
			auto prefabID = entity.GetComponent<PrefabComponent>().PrefabID;
			if (auto prefab = AssetManager::GetAsset<Prefab>(prefabID))
			{
				prefab->Create(entity, true);
				HZ_CORE_INFO("Updated prefab {}", (uint64_t)prefabID);
			}
		}
		ImGui::EndDisabled();

		if (!Project::GetActive())
			ImGui::TextDisabled("Open a project to create prefabs.");

		ImGui::Separator();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		const bool isPrefabInstance = entity.HasComponent<PrefabComponent>();
		const std::string label = isPrefabInstance ? ("[P] " + tag) : tag;

		ImGui::PushID((int)(uint32_t)entity);

		const bool selected = m_SelectionContext == entity;
		if (ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_SpanAvailWidth))
			m_SelectionContext = entity;

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			if (ImGui::MenuItem("Create Prefab..."))
				CreatePrefabFromEntity(entity);

			if (isPrefabInstance && ImGui::MenuItem("Update Prefab"))
			{
				auto prefabID = entity.GetComponent<PrefabComponent>().PrefabID;
				if (auto prefab = AssetManager::GetAsset<Prefab>(prefabID))
				{
					prefab->Create(entity, true);
					HZ_CORE_INFO("Updated prefab {}", (uint64_t)prefabID);
				}
			}

			ImGui::EndPopup();
		}

		ImGui::PopID();

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];


		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			// todo typeid????????????
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;
				ImGui::EndPopup();
			}
			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent")) {

			if (ImGui::MenuItem("Camera")) {
				m_SelectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer"))
			{
				m_SelectionContext.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Mesh Renderer"))
			{
				m_SelectionContext.AddComponent<MeshRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Static Mesh"))
			{
				m_SelectionContext.AddComponent<StaticMeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Directional Light"))
			{
				m_SelectionContext.AddComponent<DirectionalLightComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Rigid Body 2D"))
			{
				if (!m_SelectionContext.HasComponent<RigidBody2DComponent>())
					m_SelectionContext.AddComponent<RigidBody2DComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Box Collider 2D"))
			{
				if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
					m_SelectionContext.AddComponent<BoxCollider2DComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
			DrawVec3Control("Translation", component.Translation);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", component.Scale, 1.0f);
			});
		

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {

			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);
			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("Vertical FOV", &verticalFov))
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));
				float orthoNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near", &orthoNear))
					camera.SetPerspectiveNearClip(orthoNear);
				float orthoFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far", &orthoFar))
					camera.SetPerspectiveFarClip(orthoFar);
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);
				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near", &orthoNear))
					camera.SetOrthographicNearClip(orthoNear);
				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far", &orthoFar))
					camera.SetOrthographicFarClip(orthoFar);
				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
			});
		

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {

			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

			const char* preview = "None";
			std::string previewLabel;
			if (component.Texture && AssetManager::IsAssetHandleValid(component.Texture))
			{
				previewLabel = AssetManager::GetMetadata(component.Texture).FilePath.string();
				preview = previewLabel.c_str();
			}

			if (ImGui::BeginCombo("Texture", preview))
			{
				const bool noneSelected = component.Texture == AssetHandle(0);
				if (ImGui::Selectable("None", noneSelected))
					component.Texture = 0;

				for (AssetHandle handle : AssetManager::GetAllAssetsWithType(AssetType::Texture))
				{
					const auto& metadata = AssetManager::GetMetadata(handle);
					const std::string label = metadata.FilePath.string();
					const bool selected = component.Texture == handle;
					if (ImGui::Selectable(label.c_str(), selected))
						component.Texture = handle;
				}

				ImGui::EndCombo();
			}
			});

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component) {
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::Checkbox("Visible", &component.Visible);
			});

		DrawComponent<StaticMeshComponent>("Static Mesh", entity, [](auto& component) {
			const char* preview = "None";
			std::string previewLabel;
			if (component.StaticMesh && AssetManager::IsAssetHandleValid(component.StaticMesh))
			{
				previewLabel = AssetManager::GetMetadata(component.StaticMesh).FilePath.string();
				preview = previewLabel.c_str();
			}

			if (ImGui::BeginCombo("Mesh", preview))
			{
				const bool noneSelected = component.StaticMesh == AssetHandle(0);
				if (ImGui::Selectable("None", noneSelected))
					component.StaticMesh = 0;

				for (AssetHandle handle : AssetManager::GetAllAssetsWithType(AssetType::StaticMesh))
				{
					const auto& metadata = AssetManager::GetMetadata(handle);
					const std::string label = metadata.FilePath.string();
					const bool selected = component.StaticMesh == handle;
					if (ImGui::Selectable(label.c_str(), selected))
						component.StaticMesh = handle;
				}

				ImGui::EndCombo();
			}

			const char* materialPreview = "None";
			std::string materialPreviewLabel;
			if (component.Material && AssetManager::IsAssetHandleValid(component.Material))
			{
				materialPreviewLabel = AssetManager::GetMetadata(component.Material).FilePath.string();
				materialPreview = materialPreviewLabel.c_str();
			}

			if (ImGui::BeginCombo("Material", materialPreview))
			{
				const bool noneSelected = component.Material == AssetHandle(0);
				if (ImGui::Selectable("None", noneSelected))
					component.Material = 0;

				for (AssetHandle handle : AssetManager::GetAllAssetsWithType(AssetType::Material))
				{
					const auto& metadata = AssetManager::GetMetadata(handle);
					const std::string label = metadata.FilePath.string();
					const bool selected = component.Material == handle;
					if (ImGui::Selectable(label.c_str(), selected))
						component.Material = handle;
				}

				ImGui::EndCombo();
			}

			ImGui::ColorEdit4("Color Tint", glm::value_ptr(component.Color));
			ImGui::Checkbox("Visible", &component.Visible);
			});

		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component) {
			ImGui::ColorEdit3("Radiance", glm::value_ptr(component.Radiance));
			ImGui::DragFloat("Intensity", &component.Intensity, 0.05f, 0.0f, 100.0f);
			});

		DrawComponent<RigidBody2DComponent>("Rigid Body 2D", entity, [](auto& component) {
			const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
			int bodyType = (int)component.BodyType;
			if (ImGui::Combo("Body Type", &bodyType, bodyTypes, IM_ARRAYSIZE(bodyTypes)))
				component.BodyType = (RigidBody2DComponent::Type)bodyType;
			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			ImGui::DragFloat("Gravity Scale", &component.GravityScale, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Linear Damping", &component.LinearDamping, 0.001f, 0.0f, 1.0f);
			ImGui::DragFloat("Angular Damping", &component.AngularDamping, 0.001f, 0.0f, 1.0f);
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component) {
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.05f);
			ImGui::DragFloat2("Size", glm::value_ptr(component.Size), 0.05f, 0.01f);
			ImGui::DragFloat("Density", &component.Density, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.05f, 0.0f, 2.0f);
			});
		
	}

}