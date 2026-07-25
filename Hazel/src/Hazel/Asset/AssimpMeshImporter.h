#pragma once

#include "Hazel/Asset/MeshSource.h"

#include <filesystem>

namespace Hazel {

	class AssimpMeshImporter
	{
	public:
		explicit AssimpMeshImporter(const std::filesystem::path& path);

		Ref<MeshSource> ImportToMeshSource() const;

	private:
		std::filesystem::path m_Path;
	};

}
