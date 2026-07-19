#pragma once

#include "Hazel/Core/Core.h"
#include <filesystem>
#include <string>

namespace Hazel {

	class EditorAssetManager;
	class RuntimeAssetManager;

	struct ProjectConfig
	{
		std::string Name = "Untitled";
		std::string AssetDirectory = "assets";
		std::string AssetRegistryPath = "AssetRegistry.hzr";
		std::string StartScene = "scenes/Main.hazel";

		// Filled by serializer (not written to .hzproj)
		std::filesystem::path ProjectDirectory;
		std::filesystem::path ProjectFilePath;
	};

	class Project
	{
	public:
		const ProjectConfig& GetConfig() const { return m_Config; }
		ProjectConfig& GetConfig() { return m_Config; }

		std::filesystem::path GetAssetDirectory() const;
		std::filesystem::path GetAssetPath(const std::filesystem::path& relativePath) const;
		std::filesystem::path GetStartScenePath() const;
		std::filesystem::path GetAssetRegistryPath() const;

		Ref<EditorAssetManager> GetAssetManager() const { return m_AssetManager; }

		static Ref<Project> GetActive();
		static void SetActive(const Ref<Project>& project);
		static void SetActiveRuntime(const Ref<Project>& project, const Ref<class AssetPack>& assetPack);
		static void ClearActive();

		static Ref<EditorAssetManager> GetActiveAssetManager();
		static Ref<RuntimeAssetManager> GetRuntimeAssetManager();
		static bool IsRuntimeActive();
		static std::filesystem::path GetActiveAssetRegistryPath();

		static const ProjectConfig* GetActiveConfig();

		// Directory containing Hazelnut.exe (for bundled SampleProject)
		static std::filesystem::path GetRuntimeDirectory();

	private:
		ProjectConfig m_Config;
		Ref<EditorAssetManager> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
		inline static Ref<RuntimeAssetManager> s_RuntimeAssetManager;
		inline static Ref<class AssetPack> s_RuntimeAssetPack;
		inline static bool s_RuntimeActive = false;
	};

}
