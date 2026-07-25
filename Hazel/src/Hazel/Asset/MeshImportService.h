#pragma once

#include "Hazel/Asset/Asset.h"

#include <filesystem>

namespace Hazel {

	class MeshImportService
	{
	public:
		static bool IsModelExtension(const std::filesystem::path& path);
		static std::filesystem::path GetDefaultOutputPath(const std::filesystem::path& sourceModelPath);

		// Converts a model file under assets/ to .hmesh and registers MeshSource.
		static AssetHandle ImportModelAsMeshSource(const std::filesystem::path& sourceModelPath);
	};

}
