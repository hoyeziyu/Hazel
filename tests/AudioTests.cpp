#include <gtest/gtest.h>

#include "Hazel/Audio/AudioEngine.h"
#include "Hazel/Audio/AudioFileUtils.h"
#include "Hazel/Audio/SoundBank.h"
#include "Hazel/Serialization/FileStream.h"
#include "Hazel/Asset/AssetSerializer.h"
#include "Hazel/Asset/SoundConfigAsset.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <cstdlib>

TEST(AudioTest, EngineInitializesAndShutsDown)
{
	auto& engine = Hazel::AudioEngine::Get();
	engine.Init();
#ifdef HZ_AUDIO
	EXPECT_TRUE(engine.IsInitialized());
#endif
	engine.Shutdown();
}

TEST(AudioTest, AudioComponentSerializesToYaml)
{
	Hazel::Scene scene;
	Hazel::Entity entity = scene.CreateEntity("AudioEntity");
	auto& audio = entity.AddComponent<Hazel::AudioComponent>();
	audio.SoundConfig = (Hazel::AssetHandle)12345ULL;
	audio.FilePath = "Audio/click.wav";
	audio.Volume = 0.75f;
	audio.PlayOnAwake = true;
	audio.Loop = false;

	YAML::Emitter out;
	Hazel::SceneSerializer::SerializeEntity(out, entity);
	const std::string yaml = out.c_str();

	EXPECT_NE(yaml.find("AudioComponent"), std::string::npos);
	EXPECT_NE(yaml.find("SoundConfig"), std::string::npos);
	EXPECT_NE(yaml.find("Audio/click.wav"), std::string::npos);
	EXPECT_NE(yaml.find("0.75"), std::string::npos);
}

TEST(AudioTest, SoundConfigYamlRoundTrip)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "AssetID" << YAML::Value << (uint64_t)67890ULL;
	out << YAML::Key << "IsLooping" << YAML::Value << false;
	out << YAML::Key << "VolumeMultiplier" << YAML::Value << 0.8f;
	out << YAML::Key << "PitchMultiplier" << YAML::Value << 1.0f;
	out << YAML::EndMap;

	auto config = Hazel::CreateRef<Hazel::SoundConfigAsset>();
	Hazel::SoundConfigAssetSerializer serializer;
	ASSERT_TRUE(serializer.DeserializeFromYAML(out.c_str(), config));
	EXPECT_EQ((uint64_t)config->DataSourceAsset, 67890ULL);
	EXPECT_FALSE(config->IsLooping);
	EXPECT_FLOAT_EQ(config->VolumeMultiplier, 0.8f);
	EXPECT_FLOAT_EQ(config->PitchMultiplier, 1.0f);
}

TEST(AudioTest, SoundConfigYamlRoundTripLargeHandle)
{
	const uint64_t kLargeHandle = std::strtoull("10402389245123987104", nullptr, 10);
	const std::string yaml =
		"AssetID: 10402389245123987104\n"
		"IsLooping: false\n"
		"VolumeMultiplier: 0.8\n"
		"PitchMultiplier: 1.0\n";

	auto config = Hazel::CreateRef<Hazel::SoundConfigAsset>();
	Hazel::SoundConfigAssetSerializer serializer;
	ASSERT_TRUE(serializer.DeserializeFromYAML(yaml, config));
	EXPECT_EQ((uint64_t)config->DataSourceAsset, kLargeHandle);
}

TEST(AudioTest, AudioFileUtilsReadsWavHeader)
{
	namespace fs = std::filesystem;
	const fs::path tempDir = fs::temp_directory_path() / "hazel_m20_audio_test";
	fs::create_directories(tempDir);

	const fs::path wavPath = tempDir / "tone.wav";
	{
		std::ofstream out(wavPath, std::ios::binary);
		const char header[] = {
			'R','I','F','F', 36,0,0,0, 'W','A','V','E',
			'f','m','t',' ', 16,0,0,0, 1,0, 1,0,
			0x44,0xAC,0,0, 0x88,0x58,0x01,0, 2,0, 16,0,
			'd','a','t','a', 4,0,0,0, 0,0, 0,0
		};
		out.write(header, sizeof(header));
	}

	const auto info = Hazel::AudioFileUtils::GetFileInfo(wavPath);
	ASSERT_TRUE(info.has_value());
	EXPECT_EQ(info->SamplingRate, 44100u);
	EXPECT_EQ(info->NumChannels, 1u);
	EXPECT_EQ(info->BitDepth, 16u);
	EXPECT_GT(info->FileSize, 40u);
}

TEST(AudioTest, SoundBankLoadEmptyIndex)
{
	namespace fs = std::filesystem;
	const fs::path tempDir = fs::temp_directory_path() / "hazel_m20_audio_test";
	fs::create_directories(tempDir);
	const fs::path bankPath = tempDir / "EmptyBank.hsb";

	{
		Hazel::FileStreamWriter writer(bankPath);
		Hazel::SoundBankFile::FileHeader header{};
		header.AudioFileCount = 0;
		writer.WriteRaw(header);
	}

	const auto bank = Hazel::SoundBank::Load(bankPath);
	ASSERT_NE(bank, nullptr);
	EXPECT_TRUE(bank->IsLoaded());
	EXPECT_FALSE(bank->Contains(1));
}
