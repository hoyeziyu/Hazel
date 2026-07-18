#include "hzpch.h"
#include "Project.h"

#ifdef HZ_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace Hazel {

	std::filesystem::path Project::GetAssetDirectory() const
	{
		return m_Config.ProjectDirectory / m_Config.AssetDirectory;
	}

	std::filesystem::path Project::GetAssetPath(const std::filesystem::path& relativePath) const
	{
		return GetAssetDirectory() / relativePath;
	}

	std::filesystem::path Project::GetStartScenePath() const
	{
		return GetAssetPath(m_Config.StartScene);
	}

	Ref<Project> Project::GetActive()
	{
		return s_ActiveProject;
	}

	void Project::SetActive(const Ref<Project>& project)
	{
		s_ActiveProject = project;
	}

	void Project::ClearActive()
	{
		s_ActiveProject.reset();
	}

	const ProjectConfig* Project::GetActiveConfig()
	{
		return s_ActiveProject ? &s_ActiveProject->GetConfig() : nullptr;
	}

	std::filesystem::path Project::GetRuntimeDirectory()
	{
#ifdef HZ_PLATFORM_WINDOWS
		char path[MAX_PATH];
		GetModuleFileNameA(nullptr, path, MAX_PATH);
		return std::filesystem::path(path).parent_path();
#else
		return std::filesystem::current_path();
#endif
	}

}
