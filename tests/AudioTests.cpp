#include <gtest/gtest.h>

#include "Hazel/Audio/AudioEngine.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"

#include <yaml-cpp/yaml.h>

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
	audio.FilePath = "Audio/click.wav";
	audio.Volume = 0.75f;
	audio.PlayOnAwake = true;
	audio.Loop = false;

	YAML::Emitter out;
	Hazel::SceneSerializer::SerializeEntity(out, entity);
	const std::string yaml = out.c_str();

	EXPECT_NE(yaml.find("AudioComponent"), std::string::npos);
	EXPECT_NE(yaml.find("Audio/click.wav"), std::string::npos);
	EXPECT_NE(yaml.find("0.75"), std::string::npos);
}
