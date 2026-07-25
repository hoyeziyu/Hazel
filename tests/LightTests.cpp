#include <gtest/gtest.h>

#include <filesystem>

#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"

TEST(DirectionalLightTest, SerializeAndDeserializeRoundTrip)
{
	auto scene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::Entity lightEntity = scene->CreateEntity("Sun");
	const Hazel::UUID entityID = lightEntity.GetComponent<Hazel::IDComponent>().ID;
	lightEntity.GetComponent<Hazel::TransformComponent>().Rotation = glm::vec3(0.5f, 1.0f, 0.0f);
	auto& light = lightEntity.AddComponent<Hazel::DirectionalLightComponent>();
	light.Radiance = glm::vec3(0.9f, 0.8f, 0.7f);
	light.Intensity = 3.0f;

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_light_test.yaml";

	Hazel::SceneSerializer writer(scene);
	writer.Serialize(path.string());

	auto loadedScene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::SceneSerializer reader(loadedScene);
	ASSERT_TRUE(reader.Deserialize(path.string()));

	Hazel::Entity loadedLight = loadedScene->GetEntityWithUUID(entityID);
	ASSERT_TRUE(loadedLight);
	ASSERT_TRUE(loadedLight.HasComponent<Hazel::DirectionalLightComponent>());
	EXPECT_EQ(loadedLight.GetComponent<Hazel::DirectionalLightComponent>().Radiance, light.Radiance);
	EXPECT_FLOAT_EQ(loadedLight.GetComponent<Hazel::DirectionalLightComponent>().Intensity, light.Intensity);

	std::error_code ec;
	std::filesystem::remove(path, ec);
}
