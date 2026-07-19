#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Asset/Asset.h"
#include "Hazel/Project/Project.h"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Hazel {

	struct ContentBrowserItem
	{
		std::filesystem::path AbsolutePath;
		std::filesystem::path RelativePath;
		std::string DisplayName;
		AssetHandle Handle = 0;
		bool IsDirectory = false;
	};

	class ContentBrowserPanel
	{
	public:
		using SceneActivatedCallback = std::function<void(const std::filesystem::path& absolutePath)>;
		using PrefabActivatedCallback = std::function<void(AssetHandle handle)>;

		void SetSceneActivatedCallback(const SceneActivatedCallback& callback) { m_SceneActivatedCallback = callback; }
		void SetPrefabActivatedCallback(const PrefabActivatedCallback& callback) { m_PrefabActivatedCallback = callback; }

		void OnProjectChanged(const Ref<Project>& project);
		void OnImGuiRender();

	private:
		void RefreshDirectoryListing();
		std::filesystem::path GetCurrentDirectory() const;
		void SetCurrentDirectory(const std::filesystem::path& relativePath);

		void DrawToolbar();
		void DrawDirectoryTree();
		void DrawDirectoryNode(const std::filesystem::path& absolutePath, const std::filesystem::path& relativePath);
		void DrawItemList();
		void ActivateItem(const ContentBrowserItem& item);

		static bool IsHiddenEntry(const std::filesystem::path& path);
		static const char* GetTypeLabel(const std::filesystem::path& path);

	private:
		Ref<Project> m_Project;
		std::filesystem::path m_AssetRoot;
		std::filesystem::path m_CurrentRelativePath;

		std::vector<ContentBrowserItem> m_DirectoryContents;
		std::filesystem::path m_SelectedPath;

		SceneActivatedCallback m_SceneActivatedCallback;
		PrefabActivatedCallback m_PrefabActivatedCallback;
	};

}
