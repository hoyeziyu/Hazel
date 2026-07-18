#include "EditorLayer.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Debug/Instrumentor.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/Application.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Utils/PlatformUtils.h"

namespace Hazel {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
	{

	}
	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();
		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();
		// Entity
		m_SquareEntity = m_ActiveScene->CreateEntity("Green Square");
		m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });

		auto redSquare = m_ActiveScene->CreateEntity("Red Square");
		redSquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEntity.AddComponent<CameraComponent>();
		
		m_SecondCamera = m_ActiveScene->CreateEntity("Clip-Space Entity");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;


		class CameraController :public ScriptableEntity
		{
		public:
			virtual void OnCreate() override {
				auto& translation = GetComponent<TransformComponent>().Translation;
				translation.x = rand() % 10 - 5.0f;
			}

			virtual void OnDestroy() override {

			}

			virtual void OnUpdate(Timestep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				float speed = 5.0f;

				if (Input::IsKeyPressed(HZ_KEY_A))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(HZ_KEY_D))
					translation.x += speed * ts;
				if (Input::IsKeyPressed(HZ_KEY_W))
					translation.y += speed * ts;
				if (Input::IsKeyPressed(HZ_KEY_S))
					translation.y -= speed * ts;
			}
		};

		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		HZ_PROFILE_FUNCTION();

		// Resize
		if (Hazel::FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Update
		m_CameraController.OnUpdate(ts);


		// Render ?? ???????????? FBO???????????? Scene::OnUpdate ?? Renderer2D ??
		Renderer2D::ResetStats();   // ??????Stats ?????? DrawCalls vs QuadCount
		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			m_Framebuffer->Bind();    // ?????????Renderer2D ?? draw ????? FBO
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();
		}
		{
			// Scene::OnUpdate ?? BeginScene / DrawQuad??N / EndScene?????? 1 DrawCall??
			m_ActiveScene->OnUpdate(ts);

			m_Framebuffer->Unbind();
		}
	}
	void EditorLayer::OnImGuiRender()
	{
		HZ_PROFILE_FUNCTION();

		// vcpkg ??? imgui ?? Docking???? MainMenuBar + ??????????? DockSpace
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		m_SceneHierarchyPanel.OnImGuiRender();

		ImGui::Begin("Stats");
		auto stats = Renderer2D::GetStats();
		// ????Ч????Quads ??????Draw Calls ???? Flush ???????????? 1??
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		// 显示离屏 FBO 纹理；ImGui 不参与 2D 合批，只是采样已画好的结果
		ImGui::Image((ImTextureID)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();
	}
	void EditorLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}
	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// ?????
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL) || Input::IsKeyPressed(HZ_KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT) || Input::IsKeyPressed(HZ_KEY_RIGHT_SHIFT);
		switch (e.GetKeyCode())
		{
		case HZ_KEY_N:
		{
			if (control)
				NewScene();

			break;
		}
		case HZ_KEY_O:
		{
			if (control)
				OpenScene();

			break;
		}
		case HZ_KEY_S:
		{
			if (control && shift)
				SaveSceneAs();

			break;
		}
		}

		return false;
	}
	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OpenScene()
	{
		if (auto filepath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0"))
		{
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(*filepath);
		}
	}
	void EditorLayer::SaveSceneAs()
	{
		if (auto filepath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0"))
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(*filepath);
		}
	}
}