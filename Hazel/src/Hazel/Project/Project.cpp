#include "hzpch.h"
#include "Project.h"

#include "Hazel/Asset/AssetManager/EditorAssetManager.h"
#include "Hazel/Asset/AssetManager/RuntimeAssetManager.h"
#include "Hazel/Serialization/AssetPack.h"

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

	std::filesystem::path Project::GetAssetRegistryPath() const
	{
		return GetAssetDirectory() / m_Config.AssetRegistryPath;
	}

	Ref<Project> Project::GetActive()
	{
		return s_ActiveProject;
	}

	void Project::SetActive(const Ref<Project>& project)
	{
		s_RuntimeActive = false;
		s_RuntimeAssetManager.reset();
		s_RuntimeAssetPack.reset();
		s_ActiveProject = project;

		if (s_ActiveProject)
			s_ActiveProject->m_AssetManager = CreateRef<EditorAssetManager>();
	}

	void Project::SetActiveRuntime(const Ref<Project>& project, const Ref<AssetPack>& assetPack)
	{
		s_ActiveProject = project;
		s_RuntimeAssetPack = assetPack;
		s_RuntimeActive = true;
		s_RuntimeAssetManager = CreateRef<RuntimeAssetManager>();
		s_RuntimeAssetManager->SetAssetPack(assetPack);
	}

	void Project::ClearActive()
	{
		s_RuntimeActive = false;
		s_RuntimeAssetManager.reset();
		s_RuntimeAssetPack.reset();
		s_ActiveProject.reset();
	}

	const ProjectConfig* Project::GetActiveConfig()
	{
		return s_ActiveProject ? &s_ActiveProject->GetConfig() : nullptr;
	}

	Ref<EditorAssetManager> Project::GetActiveAssetManager()
	{
		return s_ActiveProject ? s_ActiveProject->GetAssetManager() : nullptr;
	}

	Ref<RuntimeAssetManager> Project::GetRuntimeAssetManager()
	{
		return s_RuntimeAssetManager;
	}

	bool Project::IsRuntimeActive()
	{
		return s_RuntimeActive;
	}

	std::filesystem::path Project::GetActiveAssetRegistryPath()
	{
		return s_ActiveProject ? s_ActiveProject->GetAssetRegistryPath() : std::filesystem::path{};
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
