#include "Panels/ContentBrowserPanel.h"

#include "Hazel/Asset/AssetExtensions.h"
#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/AssetTypes.h"
#include "Hazel/Core/Log.h"
#include "Hazel/ImGui/ImGuiUtilities.h"

#include <imgui.h>

#include <algorithm>

namespace Hazel {

	void ContentBrowserPanel::OnProjectChanged(const Ref<Project>& project)
	{
		m_Project = project;
		m_SelectedPath.clear();
		m_DirectoryContents.clear();

		if (project)
		{
			m_AssetRoot = project->GetAssetDirectory();
			m_CurrentRelativePath.clear();
			RefreshDirectoryListing();
		}
		else
		{
			m_AssetRoot.clear();
			m_CurrentRelativePath.clear();
		}
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		if (!m_Project || m_AssetRoot.empty())
			return;

		ImGui::Begin("Content Browser");

		DrawToolbar();
		ImGui::Separator();

		const float treeWidth = std::max(180.0f, ImGui::GetContentRegionAvail().x * 0.28f);
		ImGui::BeginChild("ContentBrowserTree", ImVec2(treeWidth, 0), ImGuiChildFlags_Borders);
		DrawDirectoryTree();
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginChild("ContentBrowserList", ImVec2(0, 0), ImGuiChildFlags_Borders);
		DrawItemList();
		ImGui::EndChild();

		ImGui::End();
	}

	void ContentBrowserPanel::RefreshDirectoryListing()
	{
		m_DirectoryContents.clear();

		const auto currentDirectory = GetCurrentDirectory();
		std::error_code ec;
		if (!std::filesystem::exists(currentDirectory, ec))
			return;

		std::vector<ContentBrowserItem> directories;
		std::vector<ContentBrowserItem> files;

		for (const auto& entry : std::filesystem::directory_iterator(currentDirectory, ec))
		{
			if (ec)
				break;

			if (IsHiddenEntry(entry.path()))
				continue;

			ContentBrowserItem item;
			item.AbsolutePath = entry.path();
			item.DisplayName = entry.path().filename().string();
			item.IsDirectory = entry.is_directory();
			item.RelativePath = std::filesystem::relative(item.AbsolutePath, m_AssetRoot, ec);
			if (ec)
				continue;

			if (!item.IsDirectory)
				item.Handle = AssetManager::ImportAsset(item.AbsolutePath);

			if (item.IsDirectory)
				directories.push_back(std::move(item));
			else
				files.push_back(std::move(item));
		}

		auto compareByName = [](const ContentBrowserItem& a, const ContentBrowserItem& b)
		{
			return a.DisplayName < b.DisplayName;
		};
		std::sort(directories.begin(), directories.end(), compareByName);
		std::sort(files.begin(), files.end(), compareByName);

		m_DirectoryContents.reserve(directories.size() + files.size());
		m_DirectoryContents.insert(m_DirectoryContents.end(), directories.begin(), directories.end());
		m_DirectoryContents.insert(m_DirectoryContents.end(), files.begin(), files.end());
	}

	std::filesystem::path ContentBrowserPanel::GetCurrentDirectory() const
	{
		if (m_CurrentRelativePath.empty())
			return m_AssetRoot;
		return m_AssetRoot / m_CurrentRelativePath;
	}

	void ContentBrowserPanel::SetCurrentDirectory(const std::filesystem::path& relativePath)
	{
		m_CurrentRelativePath = relativePath;
		m_SelectedPath.clear();
		RefreshDirectoryListing();
	}

	void ContentBrowserPanel::DrawToolbar()
	{
		const bool canGoUp = !m_CurrentRelativePath.empty();

		ImGui::BeginDisabled(!canGoUp);
		if (ImGui::Button("Up"))
		{
			if (canGoUp)
				SetCurrentDirectory(m_CurrentRelativePath.parent_path());
		}
		ImGui::EndDisabled();
		UI::SetTooltip("Go to parent directory");

		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
			RefreshDirectoryListing();
		UI::SetTooltip("Refresh current directory listing");

		ImGui::SameLine();
		const std::string pathLabel = m_CurrentRelativePath.empty()
			? "assets/"
			: ("assets/" + m_CurrentRelativePath.generic_string());
		ImGui::TextUnformatted(pathLabel.c_str());
	}

	void ContentBrowserPanel::DrawDirectoryTree()
	{
		ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (m_CurrentRelativePath.empty())
			rootFlags |= ImGuiTreeNodeFlags_Selected;

		const bool rootOpen = ImGui::TreeNodeEx("Assets", rootFlags);
		if (ImGui::IsItemClicked())
			SetCurrentDirectory({});

		if (rootOpen)
		{
			DrawDirectoryNode(m_AssetRoot, {});
			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawDirectoryNode(const std::filesystem::path& absolutePath, const std::filesystem::path& relativePath)
	{
		std::error_code ec;
		std::vector<std::filesystem::path> subdirectories;
		for (const auto& entry : std::filesystem::directory_iterator(absolutePath, ec))
		{
			if (ec)
				return;

			if (!entry.is_directory() || IsHiddenEntry(entry.path()))
				continue;

			subdirectories.push_back(entry.path());
		}

		std::sort(subdirectories.begin(), subdirectories.end());

		for (const auto& subdirectory : subdirectories)
		{
			const auto childRelative = relativePath / subdirectory.filename();
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (m_CurrentRelativePath == childRelative)
				flags |= ImGuiTreeNodeFlags_Selected;

			const std::string label = subdirectory.filename().string();
			const bool open = ImGui::TreeNodeEx(label.c_str(), flags);
			if (ImGui::IsItemClicked())
				SetCurrentDirectory(childRelative);

			if (open)
			{
				DrawDirectoryNode(subdirectory, childRelative);
				ImGui::TreePop();
			}
		}
	}

	void ContentBrowserPanel::DrawItemList()
	{
		if (m_DirectoryContents.empty())
		{
			ImGui::TextUnformatted("This folder is empty.");
			return;
		}

		if (ImGui::BeginTable("ContentBrowserItems", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 96.0f);
			ImGui::TableHeadersRow();

			for (const auto& item : m_DirectoryContents)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				const bool selected = m_SelectedPath == item.AbsolutePath;
				const std::string iconPrefix = item.IsDirectory ? "[DIR] " : "      ";
				const std::string label = iconPrefix + item.DisplayName;

				ImGui::PushID(item.AbsolutePath.string().c_str());
				if (ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns))
					m_SelectedPath = item.AbsolutePath;

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					ActivateItem(item);

				ImGui::TableNextColumn();
				if (item.IsDirectory)
				{
					ImGui::TextUnformatted("Folder");
				}
				else if (AssetManager::IsAssetHandleValid(item.Handle))
				{
					ImGui::TextUnformatted(std::string(AssetTypeToString(AssetManager::GetAssetType(item.Handle))).c_str());
				}
				else
				{
					ImGui::TextUnformatted(GetTypeLabel(item.AbsolutePath));
				}
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}

	void ContentBrowserPanel::ActivateItem(const ContentBrowserItem& item)
	{
		if (item.IsDirectory)
		{
			SetCurrentDirectory(item.RelativePath);
			return;
		}

		if (AssetManager::IsAssetHandleValid(item.Handle) && AssetManager::GetAssetType(item.Handle) == AssetType::Scene)
		{
			if (m_SceneActivatedCallback)
				m_SceneActivatedCallback(AssetManager::GetFileSystemPath(item.Handle));
			return;
		}

		if (AssetManager::IsAssetHandleValid(item.Handle) && AssetManager::GetAssetType(item.Handle) == AssetType::Prefab)
		{
			if (m_PrefabActivatedCallback)
				m_PrefabActivatedCallback(item.Handle);
			return;
		}

		if (item.AbsolutePath.extension() == ".hazel")
		{
			if (m_SceneActivatedCallback)
				m_SceneActivatedCallback(item.AbsolutePath);
			else
				HZ_CORE_WARN("No scene activation callback registered for Content Browser.");
		}
	}

	bool ContentBrowserPanel::IsHiddenEntry(const std::filesystem::path& path)
	{
		const auto filename = path.filename().string();
		if (filename.empty())
			return true;
		if (filename == "AssetRegistry.hzr")
			return true;
		return filename.front() == '.';
	}

	const char* ContentBrowserPanel::GetTypeLabel(const std::filesystem::path& path)
	{
		const std::string ext = path.extension().string();
		if (ext == ".hazel")
			return "Scene";
		if (ext == ".hprefab")
			return "Prefab";
		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
			return "Texture";
		if (ext == ".glsl")
			return "Shader";
		if (ext == ".ttf")
			return "Font";
		return "File";
	}

}
