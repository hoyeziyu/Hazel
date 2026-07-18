#pragma once

#include "Hazel/Core/Core.h"
#include <filesystem>
#include <string>

namespace Hazel {

	struct ProjectConfig
	{
		std::string Name = "Untitled";
		std::string AssetDirectory = "assets";
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

		static Ref<Project> GetActive();
		static void SetActive(const Ref<Project>& project);
		static void ClearActive();

		static const ProjectConfig* GetActiveConfig();

		// Directory containing Hazelnut.exe (for bundled SampleProject)
		static std::filesystem::path GetRuntimeDirectory();

	private:
		ProjectConfig m_Config;

		inline static Ref<Project> s_ActiveProject;
	};

}
