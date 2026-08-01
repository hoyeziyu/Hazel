#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"

TEST(SceneTest, CreateEntityGetsDefaultComponents)
{
	Hazel::Scene scene;

	Hazel::Entity entity = scene.CreateEntity("Player");
	ASSERT_TRUE(entity);
	EXPECT_TRUE(entity.HasComponent<Hazel::TagComponent>());
	EXPECT_TRUE(entity.HasComponent<Hazel::TransformComponent>());
	EXPECT_EQ(entity.GetComponent<Hazel::TagComponent>().Tag, "Player");
}

TEST(SceneTest, CreateEntityUsesFallbackNameWhenEmpty)
{
	Hazel::Scene scene;

	Hazel::Entity entity = scene.CreateEntity("");
	EXPECT_EQ(entity.GetComponent<Hazel::TagComponent>().Tag, "Entity");
}

TEST(SceneTest, DestroyEntityDoesNotBreakScene)
{
	Hazel::Scene scene;

	Hazel::Entity entity = scene.CreateEntity("Temp");
	ASSERT_TRUE(entity);
	scene.DestroyEntity(entity);

	Hazel::Entity another = scene.CreateEntity("Next");
	EXPECT_TRUE(another);
	EXPECT_EQ(another.GetComponent<Hazel::TagComponent>().Tag, "Next");
}

TEST(SceneTest, CopyToCopiesPrimaryCamera)
{
	Hazel::Scene editorScene;
	editorScene.OnViewportResize(800, 600);

	Hazel::Entity camera = editorScene.CreateEntity("Camera");
	camera.AddComponent<Hazel::CameraComponent>();

	auto runtimeScene = Hazel::CreateRef<Hazel::Scene>();
	editorScene.CopyTo(runtimeScene);

	ASSERT_TRUE(runtimeScene->GetPrimaryCameraEntity());
	EXPECT_EQ(runtimeScene->GetPrimaryCameraEntity().GetComponent<Hazel::TagComponent>().Tag, "Camera");
}

TEST(SceneSerializerTest, SerializeAndDeserializeRoundTrip)
{
	auto scene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::Entity entity = scene->CreateEntity("Hero");
	entity.GetComponent<Hazel::TransformComponent>().Translation = glm::vec3(1.0f, 2.0f, 3.0f);
	entity.AddComponent<Hazel::SpriteRendererComponent>(glm::vec4(0.2f, 0.4f, 0.6f, 1.0f));

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_scene_test.yaml";

	Hazel::SceneSerializer writer(scene);
	writer.Serialize(path.string());

	{
		std::ifstream serialized(path);
		ASSERT_TRUE(serialized.good());
		const std::string content(
			(std::istreambuf_iterator<char>(serialized)),
			std::istreambuf_iterator<char>());
		EXPECT_NE(content.find("Hero"), std::string::npos);
		EXPECT_NE(content.find("Translation"), std::string::npos);
	}

	auto loadedScene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::SceneSerializer reader(loadedScene);
	EXPECT_TRUE(reader.Deserialize(path.string()));

	std::error_code ec;
	std::filesystem::remove(path, ec);
}

TEST(SceneSerializerTest, TextComponentSerializeRoundTrip)
{
	auto scene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::Entity entity = scene->CreateEntity("Label");
	auto& text = entity.AddComponent<Hazel::TextComponent>();
	text.Text = "GOAL";
	text.Color = glm::vec4(1.0f, 0.9f, 0.2f, 1.0f);
	text.OffsetY = 0.8f;

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_text_component_test.yaml";

	Hazel::SceneSerializer writer(scene);
	writer.Serialize(path.string());

	auto loadedScene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::SceneSerializer reader(loadedScene);
	ASSERT_TRUE(reader.Deserialize(path.string()));

	Hazel::Entity loaded = loadedScene->TryGetEntityWithTag("Label");
	ASSERT_TRUE(loaded);
	ASSERT_TRUE(loaded.HasComponent<Hazel::TextComponent>());

	const auto& loadedText = loaded.GetComponent<Hazel::TextComponent>();
	EXPECT_EQ(loadedText.Text, "GOAL");
	EXPECT_FLOAT_EQ(loadedText.Color.r, 1.0f);
	EXPECT_FLOAT_EQ(loadedText.OffsetY, 0.8f);

	std::error_code ec;
	std::filesystem::remove(path, ec);
}
