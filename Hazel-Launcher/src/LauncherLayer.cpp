#include "LauncherLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Utils/PlatformUtils.h"

#ifdef HZ_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <cstring>
#include <vector>

#include <imgui.h>

namespace Hazel {

	LauncherLayer::LauncherLayer()
		: Layer("Launcher")
	{
	}

	void LauncherLayer::OnAttach()
	{
		const auto defaultProject = Project::GetRuntimeDirectory() / "SampleProject";
		if (std::filesystem::is_directory(defaultProject))
		{
			strncpy_s(m_ProjectPathBuffer, defaultProject.string().c_str(), sizeof(m_ProjectPathBuffer) - 1);
			m_StatusMessage = "Ready";
		}
		else
		{
			m_StatusMessage = "Bundled SampleProject not found next to Hazel-Launcher.exe";
		}
	}

	void LauncherLayer::OnImGuiRender()
	{
		ImGui::Begin("Hazel Launcher");

		ImGui::Text("Project directory");
		ImGui::InputText("##ProjectPath", m_ProjectPathBuffer, sizeof(m_ProjectPathBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Browse..."))
		{
			if (auto filepath = FileDialogs::OpenFile("Hazel Project (*.hzproj)\0*.hzproj\0"))
			{
				const auto projectDir = std::filesystem::path(*filepath).parent_path();
				strncpy_s(m_ProjectPathBuffer, projectDir.string().c_str(), sizeof(m_ProjectPathBuffer) - 1);
			}
		}

		if (ImGui::Button("Play", ImVec2(120.0f, 0.0f)))
			LaunchRuntime();

		ImGui::Separator();
		ImGui::TextWrapped("%s", m_StatusMessage.c_str());
		ImGui::Text("Runtime: %s", GetRuntimeExecutablePath().string().c_str());

		ImGui::End();
	}

	std::filesystem::path LauncherLayer::GetRuntimeExecutablePath() const
	{
		return Project::GetRuntimeDirectory() / "Hazel-Runtime.exe";
	}

	void LauncherLayer::LaunchRuntime()
	{
#ifdef HZ_PLATFORM_WINDOWS
		const auto runtimeExe = GetRuntimeExecutablePath();
		if (!std::filesystem::exists(runtimeExe))
		{
			m_StatusMessage = "Hazel-Runtime.exe not found next to Hazel-Launcher.exe";
			HZ_CORE_ERROR("{}", m_StatusMessage);
			return;
		}

		std::filesystem::path projectPath = m_ProjectPathBuffer;
		if (!projectPath.is_absolute())
			projectPath = Project::GetRuntimeDirectory() / projectPath;

		if (!std::filesystem::is_directory(projectPath))
		{
			m_StatusMessage = "Project directory not found: " + projectPath.string();
			HZ_CORE_ERROR("{}", m_StatusMessage);
			return;
		}

		std::string commandLine = "\"" + runtimeExe.string() + "\" \"" + projectPath.string() + "\"";
		std::vector<char> commandBuffer(commandLine.begin(), commandLine.end());
		commandBuffer.push_back('\0');

		STARTUPINFOA startupInfo{};
		startupInfo.cb = sizeof(startupInfo);
		PROCESS_INFORMATION processInfo{};

		if (!CreateProcessA(
			nullptr,
			commandBuffer.data(),
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			Project::GetRuntimeDirectory().string().c_str(),
			&startupInfo,
			&processInfo))
		{
			m_StatusMessage = "CreateProcess failed for Hazel-Runtime.exe";
			HZ_CORE_ERROR("{}", m_StatusMessage);
			return;
		}

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		m_StatusMessage = "Launched Hazel-Runtime for " + projectPath.filename().string();
		HZ_CORE_INFO("{}", m_StatusMessage);
#else
		m_StatusMessage = "Hazel-Launcher only supports Windows process launch in this module.";
		HZ_CORE_WARN("{}", m_StatusMessage);
#endif
	}

}
