#include "RuntimeLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Asset/AssetManager/RuntimeAssetManager.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Project/ProjectSerializer.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Serialization/AssetPack.h"
#include "Hazel/Audio/AudioEngine.h"

#include <GLFW/glfw3.h>

namespace Hazel {

	RuntimeLayer::RuntimeLayer(int argc, char** argv)
		: Layer("Runtime")
		, m_Argc(argc)
		, m_Argv(argv)
	{
	}

	void RuntimeLayer::OnAttach()
	{
		m_SceneRenderer = CreateRef<SceneRenderer>();
		m_SceneRenderer->Init();

		const auto projectDirectory = ResolveProjectDirectory(m_Argc, m_Argv);
		if (!LoadProject(projectDirectory))
		{
			HZ_CORE_ERROR("Failed to load project from {}", projectDirectory.string());
			Application::Get().Close();
		}
	}

	void RuntimeLayer::OnDetach()
	{
		if (m_Scene)
		{
			m_Scene->OnRuntimeStop();
			m_Scene.reset();
		}

		Project::ClearActive();
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		if (!m_ProjectLoaded || !m_Scene)
			return;

		auto& window = Application::Get().GetWindow();
		const uint32_t width = window.GetWidth();
		const uint32_t height = window.GetHeight();
		if (width > 0 && height > 0)
			m_Scene->OnViewportResize(width, height);

		m_Scene->OnUpdateRuntime(ts);

		Renderer2D::ResetStats();
		RenderCommand::SetClearColor({ 0.05f, 0.05f, 0.05f, 1.0f });
		RenderCommand::Clear();

		m_Scene->OnRenderRuntime(*m_SceneRenderer, false);
	}

	void RuntimeLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {
			if (e.GetKeyCode() == GLFW_KEY_ESCAPE)
			{
				Application::Get().Close();
				return true;
			}
			return false;
		});

		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
			if (m_Scene && e.GetWidth() > 0 && e.GetHeight() > 0)
				m_Scene->OnViewportResize(e.GetWidth(), e.GetHeight());
			return false;
		});
	}

	std::filesystem::path RuntimeLayer::ResolveProjectDirectory(int argc, char** argv) const
	{
		if (argc >= 2 && argv[1] && argv[1][0] != '\0')
		{
			std::filesystem::path projectPath = argv[1];
			if (!projectPath.is_absolute())
				projectPath = Project::GetRuntimeDirectory() / projectPath;

			if (std::filesystem::is_directory(projectPath))
				return std::filesystem::absolute(projectPath);

			HZ_CORE_WARN("Project directory not found: {}", projectPath.string());
		}

		const auto defaultProject = Project::GetRuntimeDirectory() / "SampleProject";
		HZ_CORE_INFO("Using default project directory: {}", defaultProject.string());
		return defaultProject;
	}

	std::filesystem::path RuntimeLayer::FindProjectFile(const std::filesystem::path& projectDirectory) const
	{
		for (const auto& entry : std::filesystem::directory_iterator(projectDirectory))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".hzproj")
				return entry.path();
		}
		return {};
	}

	bool RuntimeLayer::LoadProject(const std::filesystem::path& projectDirectory)
	{
		if (!std::filesystem::is_directory(projectDirectory))
		{
			HZ_CORE_ERROR("Project directory does not exist: {}", projectDirectory.string());
			return false;
		}

		const auto projectFile = FindProjectFile(projectDirectory);
		if (projectFile.empty())
		{
			HZ_CORE_ERROR("No .hzproj file found in {}", projectDirectory.string());
			return false;
		}

		auto project = CreateRef<Project>();
		ProjectSerializer projectSerializer(project);
		if (!projectSerializer.Deserialize(projectFile))
			return false;

		const auto runtimeDataPath = project->GetAssetDirectory() / "Project.hdat";
		if (!projectSerializer.DeserializeRuntime(runtimeDataPath))
		{
			HZ_CORE_ERROR("Missing or invalid runtime project data: {}", runtimeDataPath.string());
			HZ_CORE_ERROR("Build Asset Pack from Hazelnut before running Hazel-Runtime.");
			return false;
		}

		const auto assetPackPath = project->GetAssetDirectory() / "AssetPack.hap";
		if (!std::filesystem::exists(assetPackPath))
		{
			HZ_CORE_ERROR("Asset pack not found: {}", assetPackPath.string());
			HZ_CORE_ERROR("Build Asset Pack from Hazelnut before running Hazel-Runtime.");
			return false;
		}

		auto assetPack = AssetPack::Load(assetPackPath);
		if (!assetPack)
		{
			HZ_CORE_ERROR("Failed to load asset pack: {}", assetPackPath.string());
			return false;
		}

		Project::SetActiveRuntime(project, assetPack);

		const auto soundBankPath = project->GetAssetDirectory() / "SoundBank.hsb";
		if (std::filesystem::exists(soundBankPath))
		{
			if (!AudioEngine::Get().LoadSoundBank(soundBankPath))
				HZ_CORE_WARN("Failed to load SoundBank at {}", soundBankPath.string());
		}
		else
		{
			HZ_CORE_WARN("SoundBank not found at {} — runtime audio may be silent", soundBankPath.string());
		}

		auto runtimeAssetManager = Project::GetRuntimeAssetManager();
		if (!runtimeAssetManager)
		{
			HZ_CORE_ERROR("Runtime asset manager unavailable.");
			return false;
		}

		const AssetHandle startScene = project->GetConfig().StartSceneHandle;
		if ((uint64_t)startScene == 0)
		{
			HZ_CORE_ERROR("Start scene handle missing in Project.hdat.");
			return false;
		}

		m_Scene = runtimeAssetManager->LoadScene(startScene);
		if (!m_Scene)
		{
			HZ_CORE_ERROR("Failed to load start scene handle {}", (uint64_t)startScene);
			return false;
		}

		auto& window = Application::Get().GetWindow();
		if (window.GetWidth() > 0 && window.GetHeight() > 0)
			m_Scene->OnViewportResize(window.GetWidth(), window.GetHeight());

		m_Scene->OnRuntimeStart();
		m_ProjectLoaded = true;

		HZ_CORE_INFO("Running project '{}' from {}", project->GetConfig().Name, projectDirectory.string());
		return true;
	}

}
