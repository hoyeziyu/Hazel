#pragma once

#include "Hazel/Core/Layer.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Editor/EditorCamera.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include <glm/glm.hpp>
#include <filesystem>
#include <optional>

namespace Hazel {

	enum class SceneState
	{
		Edit = 0, Play = 1
	};

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		EditorCamera m_EditorCamera;
		Ref<Framebuffer> m_Framebuffer;

		SceneState m_SceneState = SceneState::Edit;
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		std::optional<std::filesystem::path> m_ActiveScenePath;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		int m_GizmoType = -1;

		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		bool OnKeyPressed(KeyPressedEvent& e);
		void UI_GizmoToolbar();
		void UI_PlayToolbar();

		void OnScenePlay();
		void OnSceneStop();

		void NewScene();
		void OpenScene();
		void SaveSceneAs();
		bool LoadScene(const std::filesystem::path& filepath);

		void CreateProject();
		void OpenProject();
		void OpenProject(const std::filesystem::path& filepath);
		void SaveProject();
		void CloseProject();
		void TryOpenStartupProject();
	};
}
