#include "EditorLayer.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Debug/Instrumentor.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/Application.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Math/Math.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Project/ProjectSerializer.h"
#include "Hazel/ImGui/ImGuiUtilities.h"
#include "Hazel/Utils/PlatformUtils.h"

#include <ImGuizmo.h>
#include <fstream>

namespace Hazel {

	namespace {

		void WriteDefaultScene(const std::filesystem::path& scenePath, const std::string& sceneName)
		{
			std::filesystem::create_directories(scenePath.parent_path());

			std::ofstream out(scenePath);
			out <<
				"Scene: " << sceneName << "\n"
				"Entities:\n"
				"  - Entity: 1\n"
				"    TagComponent:\n"
				"      Tag: Green Square\n"
				"    TransformComponent:\n"
				"      Translation: [0, 0, 0]\n"
				"      Rotation: [0, 0, 0]\n"
				"      Scale: [1, 1, 1]\n"
				"    SpriteRendererComponent:\n"
				"      Color: [0, 1, 0, 1]\n"
				"  - Entity: 2\n"
				"    TagComponent:\n"
				"      Tag: Camera Entity\n"
				"    TransformComponent:\n"
				"      Translation: [0, 0, 0]\n"
				"      Rotation: [0, 0, 0]\n"
				"      Scale: [1, 1, 1]\n"
				"    CameraComponent:\n"
				"      Camera:\n"
				"        ProjectionType: 1\n"
				"        PerspectiveFOV: 0.785398185\n"
				"        PerspectiveNear: 0.01\n"
				"        PerspectiveFar: 1000\n"
				"        OrthographicSize: 10\n"
				"        OrthographicNear: -1\n"
				"        OrthographicFar: 1\n"
				"      Primary: true\n"
				"      FixedAspectRatio: false\n";
		}

	}

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f)
	{
	}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = CreateRef<Scene>();
		m_SceneHierarchyPanel.SetContext(m_EditorScene);

		m_ContentBrowserPanel.SetSceneActivatedCallback([this](const std::filesystem::path& absolutePath)
		{
			LoadScene(absolutePath);
		});

		TryOpenStartupProject();
	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
		OnSceneStop();
		CloseProject();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		HZ_PROFILE_FUNCTION();

		if (Hazel::FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			if (m_RuntimeScene)
				m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if (m_SceneState == SceneState::Edit)
		{
			m_EditorCamera.SetActive(m_ViewportFocused && m_ViewportHovered);
			m_EditorCamera.OnUpdate(ts);
			m_EditorScene->OnUpdateEditor(ts);
		}

		Renderer2D::ResetStats();
		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();
		}
		{
			if (m_SceneState == SceneState::Edit)
				m_EditorScene->OnRenderEditor(m_EditorCamera);
			else if (m_RuntimeScene)
				m_RuntimeScene->OnUpdateRuntime(ts);

			m_Framebuffer->Unbind();
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		HZ_PROFILE_FUNCTION();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();
				UI::SetTooltip("Create a new empty scene");

				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					OpenScene();
				UI::SetTooltip("Open a .hazel scene file");

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();
				UI::SetTooltip("Save the current scene to a .hazel file");

				ImGui::Separator();

				if (ImGui::MenuItem("New Project..."))
					CreateProject();
				UI::SetTooltip("Create a new project (.hzproj) with default assets layout");

				if (ImGui::MenuItem("Open Project...", "Ctrl+Shift+O"))
					OpenProject();
				UI::SetTooltip("Open a project and load its Start Scene");

				if (ImGui::MenuItem("Save Project"))
					SaveProject();
				UI::SetTooltip("Save project settings; updates Start Scene if current scene is under assets/");

				if (ImGui::MenuItem("Close Project"))
					CloseProject();
				UI::SetTooltip("Save and close the active project");

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();
				UI::SetTooltip("Close Hazelnut");

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Full-screen dock host (panels below dock into MyDockspace)
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags dockHostFlags = ImGuiWindowFlags_NoDocking;
		dockHostFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		dockHostFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, dockHostFlags);
		ImGui::PopStyleVar(3);

		ImGui::DockSpace(ImGui::GetID("MyDockspace"));
		ImGui::End();

		if (auto project = Project::GetActive())
		{
			ImGui::Begin("Project");

			const std::string nameLine = "Name: " + project->GetConfig().Name;
			UI::HoverRow(nameLine.c_str(), "Project display name");

			const std::string rootPath = project->GetConfig().ProjectDirectory.string();
			const std::string rootLine = "Root: " + rootPath;
			UI::HoverRow(rootLine.c_str(), "Project root directory\n%s", rootPath.c_str());

			const std::string assetsPath = project->GetAssetDirectory().string();
			const std::string assetsLine = "Assets: " + assetsPath;
			UI::HoverRow(assetsLine.c_str(), "Asset directory\n%s", assetsPath.c_str());

			const std::string startScenePath = project->GetStartScenePath().string();
			const std::string startSceneLine = "Start Scene: " + project->GetConfig().StartScene;
			UI::HoverRow(startSceneLine.c_str(), "Start scene (relative to assets)\n%s", startScenePath.c_str());

			if (m_ActiveScenePath)
			{
				const std::string openScenePath = m_ActiveScenePath->string();
				const std::string openSceneLine = "Open Scene: " + openScenePath;
				UI::HoverRow(openSceneLine.c_str(), "Currently open scene file\n%s", openScenePath.c_str());
			}

			const std::string stateLine = std::string("State: ") + (m_SceneState == SceneState::Edit ? "Edit" : "Play");
			UI::HoverRow(stateLine.c_str(), m_SceneState == SceneState::Edit ? "Editing the scene" : "Running a runtime copy of the editor scene");

			ImGui::End();
		}

		m_SceneHierarchyPanel.OnImGuiRender();

		if (Project::GetActive())
			m_ContentBrowserPanel.OnImGuiRender();

		ImGui::Begin("Stats");
		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		UI_GizmoToolbar();
		UI_PlayToolbar();

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((ImTextureID)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (m_SceneState == SceneState::Edit && selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			const ImVec2 windowPos = ImGui::GetWindowPos();
			const ImVec2 windowSize = ImGui::GetWindowSize();
			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			bool snap = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL);
			float snapValue = 0.5f;
			if (m_GizmoType == ImGuizmo::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::UI_GizmoToolbar()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 6, 6 });
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 8, 4 });

		if (ImGui::RadioButton("Q", m_GizmoType == -1))
			m_GizmoType = -1;
		UI::SetTooltip("Select (Q)");

		ImGui::SameLine();
		if (ImGui::RadioButton("W", m_GizmoType == ImGuizmo::TRANSLATE))
			m_GizmoType = ImGuizmo::TRANSLATE;
		UI::SetTooltip("Translate (W)");

		ImGui::SameLine();
		if (ImGui::RadioButton("E", m_GizmoType == ImGuizmo::ROTATE))
			m_GizmoType = ImGuizmo::ROTATE;
		UI::SetTooltip("Rotate (E)");

		ImGui::SameLine();
		if (ImGui::RadioButton("R", m_GizmoType == ImGuizmo::SCALE))
			m_GizmoType = ImGuizmo::SCALE;
		UI::SetTooltip("Scale (R)");

		ImGui::PopStyleVar(2);
	}

	void EditorLayer::UI_PlayToolbar()
	{
		if (m_SceneState == SceneState::Edit)
			ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 12, 4 });

		const bool isPlaying = m_SceneState == SceneState::Play;
		if (isPlaying)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.2f, 0.2f, 1.0f });

		if (ImGui::Button(isPlaying ? "Stop" : "Play"))
		{
			if (isPlaying)
				OnSceneStop();
			else
				OnScenePlay();
		}

		if (isPlaying)
			ImGui::PopStyleColor();

		UI::SetTooltip(isPlaying ? "Stop (Alt+P)" : "Play (Alt+P)");
		ImGui::PopStyleVar();
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Play)
			return;

		m_SceneState = SceneState::Play;
		m_RuntimeScene = CreateRef<Scene>();
		m_EditorScene->CopyTo(m_RuntimeScene);

		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f)
			m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		m_RuntimeScene->OnRuntimeStart();
		HZ_CORE_INFO("Entered Play mode");
	}

	void EditorLayer::OnSceneStop()
	{
		if (m_SceneState != SceneState::Play)
			return;

		m_RuntimeScene->OnRuntimeStop();
		m_SceneState = SceneState::Edit;
		m_RuntimeScene = nullptr;
		HZ_CORE_INFO("Stopped Play mode");
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (m_SceneState == SceneState::Edit)
			m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL) || Input::IsKeyPressed(HZ_KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT) || Input::IsKeyPressed(HZ_KEY_RIGHT_SHIFT);
		bool alt = Input::IsKeyPressed(HZ_KEY_LEFT_ALT) || Input::IsKeyPressed(HZ_KEY_RIGHT_ALT);

		switch (e.GetKeyCode())
		{
		case HZ_KEY_P:
			if (alt)
			{
				if (m_SceneState == SceneState::Edit)
					OnScenePlay();
				else
					OnSceneStop();
			}
			break;
		case HZ_KEY_N:
			if (control)
				NewScene();
			break;
		case HZ_KEY_O:
			if (control && shift)
				OpenProject();
			else if (control)
				OpenScene();
			break;
		case HZ_KEY_S:
			if (control && shift)
				SaveSceneAs();
			break;
		case HZ_KEY_Q:
			if (m_SceneState == SceneState::Edit && m_ViewportFocused && !ImGuizmo::IsUsing())
				m_GizmoType = -1;
			break;
		case HZ_KEY_W:
			if (m_SceneState == SceneState::Edit && m_ViewportFocused && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::TRANSLATE;
			break;
		case HZ_KEY_E:
			if (m_SceneState == SceneState::Edit && m_ViewportFocused && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::ROTATE;
			break;
		case HZ_KEY_R:
			if (m_SceneState == SceneState::Edit && m_ViewportFocused && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::SCALE;
			break;
		}

		return false;
	}

	void EditorLayer::NewScene()
	{
		OnSceneStop();
		m_EditorScene = CreateRef<Scene>();
		m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);
		m_ActiveScenePath.reset();
	}

	bool EditorLayer::LoadScene(const std::filesystem::path& filepath)
	{
		if (!std::filesystem::exists(filepath))
		{
			HZ_CORE_ERROR("Scene file not found: {0}", filepath.string());
			return false;
		}

		OnSceneStop();
		m_EditorScene = CreateRef<Scene>();
		m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);

		SceneSerializer serializer(m_EditorScene);
		if (!serializer.Deserialize(filepath.string()))
			return false;

		m_ActiveScenePath = std::filesystem::absolute(filepath);
		return true;
	}

	void EditorLayer::OpenScene()
	{
		if (auto filepath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0"))
			LoadScene(*filepath);
	}

	void EditorLayer::SaveSceneAs()
	{
		if (auto filepath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0"))
		{
			SceneSerializer serializer(m_EditorScene);
			serializer.Serialize(*filepath);
			m_ActiveScenePath = std::filesystem::absolute(*filepath);
		}
	}

	void EditorLayer::TryOpenStartupProject()
	{
		const auto startupProject = Project::GetRuntimeDirectory() / "SampleProject" / "Sample.hzproj";
		if (std::filesystem::exists(startupProject))
			OpenProject(startupProject);
	}

	void EditorLayer::CreateProject()
	{
		if (auto filepath = FileDialogs::SaveFile("Hazel Project (*.hzproj)\0*.hzproj\0"))
		{
			const std::filesystem::path projectFile = std::filesystem::absolute(*filepath);
			const std::filesystem::path projectDir = projectFile.parent_path();
			const std::string projectName = projectFile.stem().string();

			std::filesystem::create_directories(projectDir / "assets" / "scenes");
			std::filesystem::create_directories(projectDir / "assets" / "textures");
			std::filesystem::create_directories(projectDir / "assets" / "shaders");
			std::filesystem::create_directories(projectDir / "assets" / "fonts");

			const auto startSceneRelative = std::filesystem::path("scenes") / "Main.hazel";
			WriteDefaultScene(projectDir / "assets" / startSceneRelative, "Main");

			auto project = CreateRef<Project>();
			auto& config = project->GetConfig();
			config.Name = projectName;
			config.AssetDirectory = "assets";
			config.StartScene = startSceneRelative.generic_string();
			config.ProjectDirectory = projectDir;
			config.ProjectFilePath = projectFile;

			ProjectSerializer serializer(project);
			serializer.Serialize(projectFile);

			OpenProject(projectFile);
		}
	}

	void EditorLayer::OpenProject()
	{
		if (auto filepath = FileDialogs::OpenFile("Hazel Project (*.hzproj)\0*.hzproj\0"))
			OpenProject(std::filesystem::path(*filepath));
	}

	void EditorLayer::OpenProject(const std::filesystem::path& filepath)
	{
		if (!std::filesystem::exists(filepath))
		{
			HZ_CORE_ERROR("Project file not found: {0}", filepath.string());
			return;
		}

		if (Project::GetActive())
			CloseProject();

		auto project = CreateRef<Project>();
		ProjectSerializer serializer(project);
		if (!serializer.Deserialize(filepath))
			return;

		Project::SetActive(project);
		m_ContentBrowserPanel.OnProjectChanged(project);
		HZ_CORE_INFO("Opened project '{0}'", project->GetConfig().Name);

		const auto startScenePath = project->GetStartScenePath();
		if (!project->GetConfig().StartScene.empty() && std::filesystem::exists(startScenePath))
		{
			if (!LoadScene(startScenePath))
				NewScene();
		}
		else
		{
			HZ_CORE_WARN("Start scene missing, creating empty scene: {0}", startScenePath.string());
			NewScene();
		}
	}

	void EditorLayer::SaveProject()
	{
		auto project = Project::GetActive();
		if (!project)
		{
			HZ_CORE_WARN("No active project to save.");
			return;
		}

		if (m_ActiveScenePath && Project::GetActive())
		{
			const auto& assetDir = project->GetAssetDirectory();
			std::error_code ec;
			const auto relative = std::filesystem::relative(*m_ActiveScenePath, assetDir, ec);
			if (!ec)
				project->GetConfig().StartScene = relative.generic_string();
		}

		ProjectSerializer serializer(project);
		serializer.Serialize(project->GetConfig().ProjectFilePath);
		HZ_CORE_INFO("Saved project '{0}'", project->GetConfig().Name);
	}

	void EditorLayer::CloseProject()
	{
		if (!Project::GetActive())
			return;

		OnSceneStop();
		SaveProject();
		Project::ClearActive();
		m_ContentBrowserPanel.OnProjectChanged(nullptr);
		NewScene();
		HZ_CORE_INFO("Closed active project.");
	}

}
