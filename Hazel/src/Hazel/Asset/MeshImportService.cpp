#include "hzpch.h"
#include "MeshImportService.h"

#include "AssimpMeshImporter.h"
#include "AssetImporter.h"
#include "AssetManager/EditorAssetManager.h"
#include "AssetSerializer.h"
#include "Hazel/Project/Project.h"

#include <fstream>

namespace Hazel {

	bool MeshImportService::IsModelExtension(const std::filesystem::path& path)
	{
		std::string ext = path.extension().string();
		for (char& c : ext)
			c = (char)std::tolower((unsigned char)c);

		return ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb" || ext == ".dae";
	}

	std::filesystem::path MeshImportService::GetDefaultOutputPath(const std::filesystem::path& sourceModelPath)
	{
		auto project = Project::GetActive();
		if (!project)
			return {};

		const auto assetRoot = project->GetAssetDirectory();
		std::filesystem::path relative = sourceModelPath;
		if (sourceModelPath.is_absolute())
		{
			std::error_code ec;
			relative = std::filesystem::relative(sourceModelPath, assetRoot, ec);
			if (ec)
				relative = sourceModelPath.filename();
		}

		relative.replace_extension(".hmesh");
		if (relative.parent_path().empty() || relative.parent_path() == ".")
			relative = std::filesystem::path("meshes") / relative.filename();

		return project->GetAssetPath(relative);
	}

	AssetHandle MeshImportService::ImportModelAsMeshSource(const std::filesystem::path& sourceModelPath)
	{
		auto project = Project::GetActive();
		auto assetManager = project ? project->GetAssetManager() : nullptr;
		if (!project || !assetManager)
			return 0;

		if (!IsModelExtension(sourceModelPath))
			return 0;

		const auto fullSourcePath = sourceModelPath.is_absolute()
			? sourceModelPath
			: project->GetAssetPath(sourceModelPath);

		std::error_code ec;
		if (!std::filesystem::exists(fullSourcePath, ec))
		{
			HZ_CORE_ERROR("Model file not found: {}", fullSourcePath.string());
			return 0;
		}

		AssimpMeshImporter importer(fullSourcePath);
		Ref<MeshSource> meshSource = importer.ImportToMeshSource();
		if (!meshSource)
			return 0;

		meshSource->SetSourceModelPath(std::filesystem::relative(fullSourcePath, project->GetAssetDirectory()).generic_string());

		const auto outputPath = GetDefaultOutputPath(fullSourcePath);
		std::filesystem::create_directories(outputPath.parent_path(), ec);

		MeshSourceSerializer serializer;
		const std::string yaml = serializer.SerializeToYAML(meshSource);
		{
			std::ofstream out(outputPath);
			if (!out)
			{
				HZ_CORE_ERROR("Failed to write mesh: {}", outputPath.string());
				return 0;
			}
			out << yaml;
		}

		const AssetHandle handle = assetManager->ImportAsset(outputPath);
		HZ_CORE_INFO("Imported model {} -> {}", fullSourcePath.string(), outputPath.string());
		return handle;
	}

}
