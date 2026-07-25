#include <gtest/gtest.h>

#include <filesystem>

#include "Hazel/Core/Log.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Physics2D/Physics2DScene.h"

TEST(Physics2DTest, DynamicBodyFallsUnderGravity)
{
	auto scene = Hazel::CreateRef<Hazel::Scene>();

	Hazel::Entity ground = scene->CreateEntity("Ground");
	ground.GetComponent<Hazel::TransformComponent>().Translation = { 0.0f, -2.0f, 0.0f };
	ground.GetComponent<Hazel::TransformComponent>().Scale = { 10.0f, 1.0f, 1.0f };
	auto& groundRb = ground.AddComponent<Hazel::RigidBody2DComponent>();
	groundRb.BodyType = Hazel::RigidBody2DComponent::Type::Static;
	ground.AddComponent<Hazel::BoxCollider2DComponent>();

	Hazel::Entity box = scene->CreateEntity("Box");
	box.GetComponent<Hazel::TransformComponent>().Translation = { 0.0f, 5.0f, 0.0f };
	auto& boxRb = box.AddComponent<Hazel::RigidBody2DComponent>();
	boxRb.BodyType = Hazel::RigidBody2DComponent::Type::Dynamic;
	box.AddComponent<Hazel::BoxCollider2DComponent>();

	scene->OnRuntimeStart();
	ASSERT_TRUE(Hazel::Physics2DScene::IsActive(*scene));

	const float startY = box.GetComponent<Hazel::TransformComponent>().Translation.y;
	scene->OnUpdateRuntime(Hazel::Timestep(0.5f));
	const float endY = box.GetComponent<Hazel::TransformComponent>().Translation.y;

	scene->OnRuntimeStop();
	EXPECT_LT(endY, startY);
}

TEST(Physics2DTest, SerializeAndDeserializeRoundTrip)
{
	auto scene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::Entity entity = scene->CreateEntity("PhysicsEntity");
	entity.GetComponent<Hazel::TransformComponent>().Translation = { 1.0f, 2.0f, 0.0f };
	auto& rb = entity.AddComponent<Hazel::RigidBody2DComponent>();
	rb.BodyType = Hazel::RigidBody2DComponent::Type::Dynamic;
	rb.GravityScale = 2.0f;
	auto& box = entity.AddComponent<Hazel::BoxCollider2DComponent>();
	box.Size = { 1.0f, 0.5f };

	const std::filesystem::path path =
		std::filesystem::temp_directory_path() / "hazel_physics2d_test.yaml";

	Hazel::SceneSerializer writer(scene);
	writer.Serialize(path.string());

	auto loadedScene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::SceneSerializer loader(loadedScene);
	ASSERT_TRUE(loader.Deserialize(path.string()));

	Hazel::Entity loaded = loadedScene->GetEntityWithUUID(entity.GetUUID());
	ASSERT_TRUE(loaded);
	EXPECT_EQ(loaded.GetComponent<Hazel::RigidBody2DComponent>().BodyType, Hazel::RigidBody2DComponent::Type::Dynamic);
	EXPECT_FLOAT_EQ(loaded.GetComponent<Hazel::RigidBody2DComponent>().GravityScale, 2.0f);
	EXPECT_FLOAT_EQ(loaded.GetComponent<Hazel::BoxCollider2DComponent>().Size.x, 1.0f);

	std::error_code ec;
	std::filesystem::remove(path, ec);
}
