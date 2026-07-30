#include <gtest/gtest.h>

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Prefab.h"
#include "Hazel/Scene/Components.h"

TEST(HierarchyTest, DeserializeParentChildrenAndPositionAlias)
{
	const std::string yaml = R"(
Prefab:
  - Entity: 100
    TagComponent:
      Tag: Root
    Parent: 0
    Children:
      - 200
    TransformComponent:
      Position: [1, 2, 3]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
  - Entity: 200
    TagComponent:
      Tag: Child
    Parent: 100
    Children: []
    TransformComponent:
      Translation: [4, 5, 6]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
)";

	auto prefab = Hazel::CreateRef<Hazel::Prefab>();
	ASSERT_TRUE(prefab->LoadFromYAML(yaml));
	ASSERT_TRUE(prefab->GetRootEntity());

	EXPECT_EQ(prefab->GetRootEntity().GetComponent<Hazel::TagComponent>().Tag, "Root");
	EXPECT_FLOAT_EQ(prefab->GetRootEntity().GetComponent<Hazel::TransformComponent>().Translation.x, 1.0f);

	auto child = prefab->GetScene()->GetEntityWithUUID(200);
	ASSERT_TRUE(child);
	EXPECT_EQ((uint64_t)child.GetComponent<Hazel::HierarchyComponent>().Parent, 100ull);
	EXPECT_EQ(prefab->GetRootEntity().GetComponent<Hazel::HierarchyComponent>().Children.size(), 1u);
	EXPECT_EQ((uint64_t)prefab->GetRootEntity().GetComponent<Hazel::HierarchyComponent>().Children[0], 200ull);
}

TEST(HierarchyTest, InstantiateRemapsHierarchy)
{
	const std::string yaml = R"(
Prefab:
  - Entity: 100
    TagComponent:
      Tag: Root
    Parent: 0
    Children:
      - 200
    PrefabComponent:
      Prefab: 999
      Entity: 100
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
  - Entity: 200
    TagComponent:
      Tag: Child
    Parent: 100
    Children: []
    PrefabComponent:
      Prefab: 999
      Entity: 200
    TransformComponent:
      Translation: [1, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
)";

	auto prefab = Hazel::CreateRef<Hazel::Prefab>();
	ASSERT_TRUE(prefab->LoadFromYAML(yaml));

	auto scene = Hazel::CreateRef<Hazel::Scene>();
	const glm::vec3 spawnPos = { 10.0f, 0.0f, 0.0f };
	Hazel::Entity instance = scene->Instantiate(prefab, &spawnPos, nullptr, nullptr);
	ASSERT_TRUE(instance);
	EXPECT_EQ(instance.GetComponent<Hazel::TagComponent>().Tag, "Root");
	EXPECT_FLOAT_EQ(instance.GetComponent<Hazel::TransformComponent>().Translation.x, 10.0f);
	EXPECT_EQ(scene->GetAllEntityUUIDs().size(), 2u);

	Hazel::UUID childUuid = instance.GetComponent<Hazel::HierarchyComponent>().Children.front();
	auto child = scene->GetEntityWithUUID(childUuid);
	ASSERT_TRUE(child);
	EXPECT_EQ(child.GetComponent<Hazel::TagComponent>().Tag, "Child");
	EXPECT_EQ((uint64_t)child.GetComponent<Hazel::HierarchyComponent>().Parent, (uint64_t)instance.GetUUID());
	EXPECT_NE((uint64_t)child.GetUUID(), 200ull);
}

TEST(HierarchyTest, Ld51GrassTilePrefabLoadsMesh)
{
	const std::string yaml = R"(
Prefab:
  - Entity: 10069443060331666450
    TagComponent:
      Tag: GrassTile
    Parent: 0
    Children: []
    PrefabComponent:
      Prefab: 20402389245123987110
      Entity: 10069443060331666450
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    StaticMeshComponent:
      StaticMesh: 18402389245123987102
      Material: 19402389245123987103
      Color: [0.2, 0.8, 0.2, 1]
      Visible: true
)";

	auto prefab = Hazel::CreateRef<Hazel::Prefab>();
	ASSERT_TRUE(prefab->LoadFromYAML(yaml));

	auto scene = Hazel::CreateRef<Hazel::Scene>();
	Hazel::Entity instance = scene->Instantiate(prefab, nullptr, nullptr, nullptr);
	ASSERT_TRUE(instance);
	EXPECT_EQ(instance.GetComponent<Hazel::TagComponent>().Tag, "GrassTile");
	EXPECT_TRUE(instance.HasComponent<Hazel::StaticMeshComponent>());
	EXPECT_EQ((uint64_t)instance.GetComponent<Hazel::StaticMeshComponent>().StaticMesh, 18402389245123987102ull);
}
