#include <gtest/gtest.h>

#include "Hazel/Asset/AssetManager.h"
#include "Hazel/Asset/AssetManager/RuntimeAssetManager.h"
#include "Hazel/Project/Project.h"
#include "Hazel/Project/ProjectRuntimeFormat.h"
#include "Hazel/Project/ProjectSerializer.h"
#include "Hazel/Serialization/FileStream.h"
#include "Hazel/Serialization/AssetPack.h"
#include "Hazel/Audio/AudioEngine.h"
#include "Hazel/Asset/SoundConfigAsset.h"

#include <cstring>
#include <cstdlib>
#include <filesystem>

TEST(RuntimeTest, ProjectRuntimeInfoRoundTrip)
{
	Hazel::ProjectRuntimeInfo runtimeInfo{};
	runtimeInfo.StartScene = 11703174775267464727ull;

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_runtime_test.hdat";

	{
		Hazel::FileStreamWriter writer(path);
		ASSERT_TRUE(writer);
		writer.WriteRaw(runtimeInfo);
		ASSERT_TRUE(writer.IsStreamGood());
	}

	Hazel::ProjectRuntimeInfo loaded{};
	{
		Hazel::FileStreamReader reader(path);
		ASSERT_TRUE(reader);
		reader.ReadRaw(loaded);
		ASSERT_TRUE(reader.IsStreamGood());
	}

	EXPECT_EQ(std::memcmp(loaded.Header.HEADER, "HDAT", 4), 0);
	EXPECT_EQ(loaded.Header.Version, 1u);
	EXPECT_EQ((uint64_t)loaded.StartScene, 11703174775267464727ull);

	std::filesystem::remove(path);
}

TEST(RuntimeTest, DeserializeRuntimeUpdatesStartSceneHandle)
{
	auto project = Hazel::CreateRef<Hazel::Project>();
	Hazel::ProjectSerializer serializer(project);

	Hazel::ProjectRuntimeInfo runtimeInfo{};
	runtimeInfo.StartScene = 42;

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_runtime_deserialize_test.hdat";

	{
		Hazel::FileStreamWriter writer(path);
		ASSERT_TRUE(writer);
		writer.WriteRaw(runtimeInfo);
	}

	ASSERT_TRUE(serializer.DeserializeRuntime(path));
	EXPECT_EQ((uint64_t)project->GetConfig().StartSceneHandle, 42ull);

	std::filesystem::remove(path);
}

TEST(RuntimeTest, AssetManagerRoutesToRuntimeManager)
{
	const uint64_t kClickWavHandle = std::strtoull("10402389245123987104", nullptr, 10);
	const uint64_t kSoundConfigHandle = std::strtoull("11402389245123987105", nullptr, 10);

	const auto repoRoot = std::filesystem::path(HAZEL_REPO_ROOT);
	const auto sampleProject = repoRoot / "Hazelnut" / "SampleProject";
	const auto assetPackPath = sampleProject / "assets" / "AssetPack.hap";
	if (!std::filesystem::exists(assetPackPath))
		GTEST_SKIP() << "SampleProject AssetPack.hap missing — run BuildSamplePack first";

	auto project = Hazel::CreateRef<Hazel::Project>();
	Hazel::ProjectSerializer projectSerializer(project);
	const auto projectFile = sampleProject / "Sample.hzproj";
	ASSERT_TRUE(projectSerializer.Deserialize(projectFile));
	ASSERT_TRUE(projectSerializer.DeserializeRuntime(sampleProject / "assets" / "Project.hdat"));

	auto assetPack = Hazel::AssetPack::Load(assetPackPath);
	ASSERT_TRUE(assetPack);

	Hazel::Project::SetActiveRuntime(project, assetPack);

	EXPECT_TRUE(Hazel::Project::IsRuntimeActive());
	EXPECT_TRUE(Hazel::AssetManager::IsAssetHandleValid(project->GetConfig().StartSceneHandle));

	auto scene = Hazel::Project::GetRuntimeAssetManager()->LoadScene(project->GetConfig().StartSceneHandle);
	ASSERT_NE(scene, nullptr);

	const auto soundBankPath = sampleProject / "assets" / "SoundBank.hsb";
	ASSERT_TRUE(std::filesystem::exists(soundBankPath)) << "SampleProject SoundBank.hsb missing — run BuildSamplePack first";
	EXPECT_TRUE(Hazel::AudioEngine::Get().LoadSoundBank(soundBankPath));
	EXPECT_TRUE(Hazel::AudioEngine::Get().GetSoundBank()->Contains(kClickWavHandle));

	const auto soundConfig = Hazel::AssetManager::GetAsset<Hazel::SoundConfigAsset>(kSoundConfigHandle);
	ASSERT_NE(soundConfig, nullptr);
	EXPECT_EQ((uint64_t)soundConfig->DataSourceAsset, kClickWavHandle);

	Hazel::Project::ClearActive();
	EXPECT_FALSE(Hazel::Project::IsRuntimeActive());
}
