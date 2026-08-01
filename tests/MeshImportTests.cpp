#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Asset/AssimpMeshImporter.h"
#include "Hazel/Asset/MeshImportService.h"

namespace {

	std::filesystem::path WriteTempObjCube()
	{
		const std::filesystem::path path =
			std::filesystem::temp_directory_path() / "hazel_assimp_cube.obj";

		std::ofstream out(path);
		out <<
			"v -0.5 -0.5 0.5\n"
			"v 0.5 -0.5 0.5\n"
			"v 0.5 0.5 0.5\n"
			"v -0.5 0.5 0.5\n"
			"f 1 2 3\n"
			"f 1 3 4\n";
		return path;
	}

}

TEST(MeshImportTest, AssimpImportsObjGeometry)
{
	const std::filesystem::path objPath = WriteTempObjCube();

	Hazel::AssimpMeshImporter importer(objPath);
	Hazel::Ref<Hazel::MeshSource> mesh = importer.ImportToMeshSource();
	ASSERT_TRUE(mesh);
	EXPECT_GE(mesh->GetPositions().size(), 4u);
	EXPECT_GE(mesh->GetIndices().size(), 3u);
	EXPECT_EQ(mesh->GetPositions().size(), mesh->GetNormals().size());

	std::error_code ec;
	std::filesystem::remove(objPath, ec);
}

TEST(MeshImportTest, DetectsModelExtensions)
{
	EXPECT_TRUE(Hazel::MeshImportService::IsModelExtension("foo.obj"));
	EXPECT_TRUE(Hazel::MeshImportService::IsModelExtension("bar.fbx"));
	EXPECT_FALSE(Hazel::MeshImportService::IsModelExtension("tex.png"));
}

TEST(MeshImportTest, Ld51TileFlatObjImports)
{
	const std::filesystem::path objPath =
		std::filesystem::path(HAZEL_REPO_ROOT) / "Hazelnut/LD51Project/assets/models/TileFlat.obj";

	if (!std::filesystem::exists(objPath))
		GTEST_SKIP() << "LD51 TileFlat.obj not found";

	Hazel::AssimpMeshImporter importer(objPath);
	Hazel::Ref<Hazel::MeshSource> mesh = importer.ImportToMeshSource();
	ASSERT_TRUE(mesh);
	EXPECT_GE(mesh->GetPositions().size(), 4u);
	EXPECT_GE(mesh->GetIndices().size(), 3u);
}
